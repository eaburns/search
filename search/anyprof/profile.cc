#include "profile.hpp"
#include "../../utils/utils.hpp"
#include <limits>

AnytimeProfile::AnytimeProfile(unsigned int cb, unsigned int tb,
		const std::vector<SolutionStream> &stream) {
	initbins(cb, tb);
	seesolutions(stream);
}

AnytimeProfile::AnytimeProfile(FILE *in) {
	if (fscanf(in, "%u %u\n", &ncost, &ntime) != 2)
		fatal("failed to read number of bins");

	if (fscanf(in, "%lg %lg\n", &mincost, &maxcost) != 2)
		fatal("failed to read min and max cost");
	if (fscanf(in, "%lg %lg\n", &mintime, &maxtime) != 2)
		fatal("failed to read min and max time");

	initbins(ncost, ntime);
	for (unsigned int i = 0; i < ncost*ntime; i++) {
		if (fscanf(in, "%u\n", qtcounts+i) != 1)
			fatal("failed to read %uth qtcount", i);
	}
	for (unsigned int i = 0; i < ncost*ncost*ntime; i++) {
		if (fscanf(in, "%u\n", qqtcounts+i) != 1)
			fatal("failed to read the %uth qqtcount");
	}
	for (unsigned int i = 0; i < ncost*ncost*ntime; i++) {
		if (fscanf(in, "%lg\n", qqtprobs+i) != 1)
			fatal("failed to read the %uth qqtprob");
	}
}

AnytimeProfile::~AnytimeProfile() {
	delete []qtcounts;
	delete []qqtcounts;
	delete []qqtprobs;
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
		fprintf(out, "%g\n", qqtprobs[i]);
}

double AnytimeProfile::prob(double qj, double qi, double dt) const {
	if (qj < mincost || qj > maxcost || qi < mincost || qi > maxcost || dt < mintime || dt > maxtime)
		return 0.0;

	unsigned int qjbin = costbin(qj);
	unsigned int qibin = costbin(qi);
	unsigned int dtbin = timebin(dt);

	assert(qjbin < ncost);
	assert(qibin < ncost);
	assert(dtbin < ntime);

	return qqtprobs[qqtindex(qjbin, qibin, dtbin)];
}

void AnytimeProfile::initbins(unsigned int cb, unsigned int tb) {
	ncost = cb;
	ntime = tb;

	qtcounts = new unsigned int[ncost*ntime];
	for (unsigned int i = 0; i < ncost*ntime; i++)
		qtcounts[i] = 0;

	qqtcounts = new unsigned int[ncost*ncost*ntime];
	qqtprobs = new double[ncost*ncost*ntime];
	for (unsigned int i = 0; i < ncost*ncost*ntime; i++) {
		qqtcounts[i] = 0;
		qqtprobs[i] = 0;
	}
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

			for (unsigned int j = i; j < stream.size(); j++) {
				unsigned int dt = timebin(stream[j].time - stream[i].time);
				unsigned int qj = costbin(stream[j].cost);
				qtcounts[qtindex(qi, dt)]++;
				qqtcounts[qqtindex(qj, qi, dt)]++;
			}
		}
	}
}

void AnytimeProfile::mkprobs() {
	for (unsigned int qi = 0; qi < ncost; qi++) {
	for (unsigned int dt = 0; dt < ntime; dt++) {
		unsigned int n = qtcounts[qtindex(qi, dt)];
		if (n == 0)
			continue;

		for (unsigned int qj = qi; qj < ncost; qj++) {
			unsigned int i = qqtindex(qj, qi, dt);
			unsigned int m = qqtcounts[i];

			// solutions must be improving.
			assert (qj >= qi || m == 0);

			qqtprobs[i] = m / n;
		}
	}
	}
	// normalize
	for (unsigned int qi = 0; qi < ncost; qi++) {
	for (unsigned int dt = 0; dt < ntime; dt++) {
		double sum = 0;

		for (unsigned int qj = qi; qj < ncost; qj++)
			sum += qqtprobs[qqtindex(qj, qi, dt)];

		if (sum == 0)
			continue;

		for (unsigned int qj = qi; qj < ncost; qj++)
			qqtprobs[qqtindex(qj, qi, dt)] /= sum;
	}
	}
}


AnytimeMonitor::AnytimeMonitor(const AnytimeProfile &p, double fw, double tw) :
	wf(fw), wt(tw), profile(p) {

}