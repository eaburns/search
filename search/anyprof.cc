#include "anyprof.hpp"
#include "../utils/utils.hpp"
#include <cmath>
#include <cstring>

AnyProf::AnyProf(const std::vector<Imp> &imps, int qb, int tb) :
		qmax(0), dtmax(0), tmax(0), qbins(qb), dtbins(tb) {
	prof.resize(qb*qb*tb, 0);
	maxes(imps);
	qwidth = ceil(qmax) / qbins;
	twidth = ceil(dtmax) / dtbins;

	std::vector<unsigned int> cnt;
	cnt.resize(qbins*dtbins, 0);

	// counts for each bin
	for (unsigned int i = 0; i < imps.size(); i++) {
		const struct Imp &im = imps[i];
		double dt = im.t1 - im.t0;
		cnt.at((im.q0/qwidth)*dtbins + dt/twidth)++;
		prof.at(index(im.q0, im.q1, dt))++;
	}

	// normalize bins
	for (int q0 = 0; q0 < qbins; q0++) {
	for (int dt = 0; dt < dtbins; dt++) {
		int ci = q0*dtbins + dt;
		if (cnt.at(ci) == 0)
			continue;

		int base = q0*dtbins*qbins + dt*qbins;
		for (int q1 = 0; q1 < qbins; q1++)
			prof.at(base + q1) /= cnt[ci];
	}
	}
}

void AnyProf::write(FILE *out) const {
	if (fprintf(out, "qmax: %g\n", qmax) < 0 ||
			fprintf(out, "dtmax: %g\n", dtmax) < 0 ||
			fprintf(out, "tmax: %g\n", tmax) < 0 ||
			fprintf(out, "qbins: %d\n", qbins) < 0 ||
			fprintf(out, "dtbins: %d\n", dtbins) < 0 ||
			fprintf(out, "size: %u\n", (unsigned int) prof.size()) < 0)
		fatal("failed to write anytime profile");

	if (fprintf(out, "%g", prof[0]) < 0)
		fatal("failed to write anytime profile");
	for (unsigned int i = 1; i < prof.size(); i++) {
		if (fprintf(out, " %g", prof[i]) < 0)
			fatal("failed to write anytime profile");
	}
}

void AnyProf::read(FILE *in) {
	unsigned int sz;
	if (fscanf(in, "qmax: %lg\n", &qmax) != 1 ||
			fscanf(in, "dtmax: %lg\n", &dtmax) != 1 ||
			fscanf(in, "tmax: %lg\n", &tmax) != 1 ||
			fscanf(in, "qbins: %d\n", &qbins) != 1 ||
			fscanf(in, "dtbins: %d\n", &dtbins) != 1 ||
			fscanf(in, "size: %u\n", &sz) != 1)
		fatal("failed to read anytime profile");

	qwidth = ceil(qmax) / qbins;
	twidth = ceil(dtmax) / dtbins;

	prof.clear();
	prof.resize(sz);
	for (unsigned int i = 0; i < sz; i++) {
		if (fscanf(in, " %lg", &prof[i]) != 1)
			fatal("failed to read anytime profile");
	}
}

void AnyProf::maxes(const std::vector<Imp> &imps) {
	for (unsigned int i = 0; i < imps.size(); i++) {
		const struct Imp &im = imps[i];

		if (im.q0 > qmax)
			qmax = im.q0;
		assert (im.q1 <= im.q0);

		if (im.t1 > tmax)
			tmax = im.t1;

		double dt = im.t1 - im.t0;
		assert (dt > 0);
		if (dt > dtmax)
			dtmax = dt;
	}
}

MonPolicy::MonPolicy(const AnyProf &p, double _wq, double _wt) :
		wq(_wq), wt(_wt), prof(p),
		qbins(ceil(p.qmax) / p.qbins),
		tbins(ceil(p.tmax) / p.twidth) {
	v.resize(qbins*tbins, 0);

	for (int qi = 0; qi < qbins; qi++) {
		double q0 = qi*prof.qwidth;
		for (int ti = tbins - 1; ti >= 0; ti--) {
			double t = ti * prof.twidth;
			bool smax = false;
			double dtmax = 0;
			double max = 0;
		
			// max over Δt,m
			for (int dti = 0; dti < prof.dtbins; dti++) {
				double dt = dti*prof.twidth;
				double vstop = usum(q0, dt, t + dt);
				double vgo = vsum(q0, dt, t + dt);
				if (vstop > max) {
					max = vstop;
					smax = true;
					dtmax = dt;
				}
				if (vgo > max) {
					max = vgo;
					smax = false;
					dtmax = dt;
				}
			}
			v.at(qi*tbins + ti) = max;
			stop.at(qi*tbins + ti) = smax;
			delta.at(qi*tbins + ti) = dtmax;
		}
	}
}

double MonPolicy::usum(double q0, double dt, double t1) const {
	if (t1 >= prof.tmax)
		return 0;

	double sum = 0;
	for (int j = 0; j < prof.qbins; j++) {
		double q1 = j*prof.qwidth;
		sum += prof.pr(q0, q1, dt) * -(q1*wq + t1*wt);
	}

	return sum;
}

double MonPolicy::vsum(double q0, double dt, double t1) const {
	if (t1 >= prof.tmax)
		return 0;

	double sum = 0;
	for (int j = 0; j < prof.qbins; j++) {
		double q1 = j*prof.qwidth;
		unsigned int i = index(q1, t1);
		if (i > v.size());
			continue;
		sum += prof.pr(q0, q1, dt) * v.at(i);
	}

	return sum;
}