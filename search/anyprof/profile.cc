// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "profile.hpp"
#include "../../utils/utils.hpp"
#include <limits>
#include <errno.h>

AnytimeProfile::AnytimeProfile(unsigned int cb, unsigned int tb,
		const std::vector<SolutionStream> &stream) {
	initbins(cb, tb);
	seesolutions(stream);
}

AnytimeProfile::AnytimeProfile(const std::string &path) {
	FILE *f = fopen(path.c_str(), "r");
	if (!f)
		fatalx(errno,"failed to open %s for reading", path.c_str());
	read(f);
	fclose(f);
}

AnytimeProfile::AnytimeProfile(FILE *in) {
	read(in);
}

void AnytimeProfile::read(FILE *in) {
	if (fscanf(in, "%u %u\n", &ncost, &ntime) != 2)
		fatal("failed to read number of bins");

	if (fscanf(in, "%lg %lg\n", &mincost, &maxcost) != 2)
		fatal("failed to read min and max cost");
	if (fscanf(in, "%lg %lg\n", &mintime, &maxtime) != 2)
		fatal("failed to read min and max time");

	initbins(ncost, ntime);
	for (unsigned int i = 0; i < ncost*ntime; i++) {
		if (fscanf(in, "%u\n", &qtcounts[i]) != 1)
			fatal("failed to read %uth qtcount", i);
	}
	for (unsigned int i = 0; i < ncost*ncost*ntime; i++) {
		if (fscanf(in, "%u\n", &qqtcounts[i]) != 1)
			fatal("failed to read the %uth qqtcount");
	}
	for (unsigned int i = 0; i < ncost*ncost*ntime; i++) {
		if (fscanf(in, "%lg\n", &qqtprobs[i]) != 1)
			fatal("failed to read the %uth qqtprob");
	}
}

void AnytimeProfile::save(FILE *out) const {
	fprintf(out, "%u %u\n", ncost, ntime);
	fprintf(out, "%g %g\n", mincost, maxcost);
	fprintf(out, "%g %g\n", mintime, maxtime);
	for (unsigned int i = 0; i < ncost*ntime; i++)
		fprintf(out, "%u\n", qtcounts[i]);
	for (unsigned int i = 0; i < ncost*ncost*ntime; i++)
		fprintf(out, "%u\n", qqtcounts[i]);
	for (unsigned int i = 0; i < ncost*ncost*ntime; i++)
		fprintf(out, "%15.15g\n", qqtprobs[i]);
}

void AnytimeProfile::initbins(unsigned int cb, unsigned int tb) {
	ncost = cb;
	ntime = tb;
	qtcounts.resize(ncost*ntime, 0);
	qqtcounts.resize(ncost*ncost*ntime, 0);
	qqtprobs.resize(ncost*ncost*ntime, 0);
}

void AnytimeProfile::seesolutions(const std::vector<SolutionStream> &streams) {
	getextents(streams);
	countsolutions(streams);
	mkprobs();
}

void AnytimeProfile::getextents(const std::vector<SolutionStream> &streams) {
	mintime = std::numeric_limits<double>::infinity();
	maxtime = -std::numeric_limits<double>::infinity();
	mincost = std::numeric_limits<double>::infinity();
	maxcost = -std::numeric_limits<double>::infinity();
	for (auto stream = streams.begin(); stream != streams.end(); stream++) {
	for (auto sol = stream->begin(); sol != stream->end(); sol++) {
		if (sol->cost < mincost)
			mincost = sol->cost;
		if (sol->cost > maxcost)
			maxcost = sol->cost;
		if (sol->time < mintime)
			mintime = sol->time;
		if (sol->time > maxtime)
			maxtime = sol->time;
	}
	}
}

void AnytimeProfile::countsolutions(const std::vector<SolutionStream> &streams) {
	for (auto s = streams.begin(); s != streams.end(); s++) {
		const SolutionStream &stream = *s;

		for (unsigned int i = 0; i < stream.size(); i++) {
			unsigned int qi = costbin(stream[i].cost);
			unsigned int curq = qi;
			unsigned int curdt = 0;
	
			for (unsigned int j = i+1; j < stream.size(); j++) {
				assert(stream[j].time > stream[i].time);
				assert(stream[j].cost <= stream[i].cost);
	
				unsigned int nextq = costbin(stream[j].cost);
				unsigned int nextdt = timebin(stream[j].time - stream[i].time);
	
				for (unsigned int dt = curdt; dt < nextdt; dt++) {
					qtcounts[qtindex(qi, dt)]++;
					qqtcounts[qqtindex(curq, qi, dt)]++;
				}
	
				curq = nextq;
				curdt = nextdt;
			}
	
			for (unsigned int dt = curdt; dt < ntime; dt++) {
				qtcounts[qtindex(qi, dt)]++;
				qqtcounts[qqtindex(curq, qi, dt)]++;
			}
		}
	}
}

void AnytimeProfile::mkprobs() {
	double small = std::numeric_limits<double>::infinity();

	for (unsigned int qi = 0; qi < ncost; qi++) {
	for (unsigned int dt = 0; dt < ntime; dt++) {
		unsigned int n = qtcounts[qtindex(qi, dt)];
		if (n == 0)
			continue;

		for (unsigned int qj = 0; qj < ncost; qj++) {
			unsigned int i = qqtindex(qj, qi, dt);
			unsigned int m = qqtcounts[i];

			// solutions must be improving.
			assert (qj <= qi || m == 0);

			qqtprobs[i] = m / (double) n;

			if (qqtprobs[i] > 0 && qqtprobs[i] < small)
				small = qqtprobs[i];
		}
	}
	}

	// smoothing
	printf("Adding %g for smoothing\n", small/2);
	for (unsigned int qi = 0; qi < ncost; qi++) {
	for (unsigned int dt = 0; dt < ntime; dt++) {
	for (unsigned int qj = 0; qj <= qi; qj++) {
		qqtprobs[qqtindex(qj, qi, dt)] += small/2;
	}
	}
	}

	// normalize
	for (unsigned int qi = 0; qi < ncost; qi++) {
	for (unsigned int dt = 0; dt < ntime; dt++) {
		double sum = 0;

		for (unsigned int qj = 0; qj < ncost; qj++)
			sum += qqtprobs[qqtindex(qj, qi, dt)];

		assert (sum > 0);

		for (unsigned int qj = 0; qj <= qi; qj++)
			qqtprobs[qqtindex(qj, qi, dt)] /= sum;

		// Some sanity checks.
		sum = 0;
		for (unsigned int qj = 0; qj <= qi; qj++) {
			sum += qqtprobs[qqtindex(qj, qi, dt)];
			assert (qj <= qi || qqtprobs[qqtindex(qj, qi, dt)] == 0);
		}
		assert (sum > 1-1e-8);
		assert (sum < 1+1e-8);
	}
	}
}

AnytimeMonitor::AnytimeMonitor(const AnytimeProfile &p, double fw, double tw) :
	prof(p), wf(fw), wt(tw),
	qwidth((p.maxcost - p.mincost)/p.ncost),
	twidth((p.maxtime - p.mintime)/p.ntime) {

	policy.resize(prof.ncost*prof.ntime);
	value.resize(prof.ncost*prof.ntime);
	seen.resize(prof.ncost*prof.ntime, false);

	// max time
	for (unsigned int q = 0; q < prof.ncost; q++) {
		unsigned int i = index(q, prof.ntime-1);
		value[i] = binutil(q, prof.ntime-1);
		seen[i] = true;
		policy[i] = true;
	}

	// min cost
	for (unsigned int t = 0; t < prof.ntime; t++) {
		unsigned int i = index(0, t);
		value[i] = binutil(0, t);
		seen[i] = true;
		policy[i] = true;
	}

	for (unsigned int q1 = 1; q1 < prof.ncost; q1++) {
	for (long t = prof.ntime-2; t >= 0; t--) {

		double vgo = 0;
		double psum = 0;
		for (long q2 = q1; q2 >= 0; q2--) {
			double prob = prof.binprob(q2, q1, 1);
			psum += prob;
			double val = value[index(q2, t+1)];
 			assert (seen[index(q2, t+1)]);
			vgo += prob*val;
		}

		assert (psum > 1 - 1e-8);
		assert (psum < 1 + 1e-8);

		if (psum == 0)
			vgo = -std::numeric_limits<double>::infinity();

		double vstop = binutil(q1, t);

		unsigned int i = index(q1, t);
		if (vgo > vstop) {
			value[i] = vgo;
			policy[i] = false;
		} else {
			value[i] = vstop;
			policy[i] = true;
		}
		seen[i] = true;
	}
	}
}
