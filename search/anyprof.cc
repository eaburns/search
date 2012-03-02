#include "anyprof.hpp"
#include <cmath>
#include <cstring>

AnyProf::AnyProf(const std::vector<Imp> &imps, int cb, int tb) :
		cmax(0), dtmax(0), tmax(0), cbins(cb), tbins(tb) {
	prof.resize(cb*cb*tb, 0);
	maxes(imps);
	cwidth = ceil(cmax) / cbins;
	twidth = ceil(dtmax) / tbins;

	std::vector<unsigned int> cnt;
	cnt.resize(cbins*tbins, 0);

	// counts for each bin
	for (unsigned int i = 0; i < imps.size(); i++) {
		const struct Imp &im = imps[i];
		double dt = im.t1 - im.t0;
		cnt.at((im.c0/cwidth)*tbins + dt/twidth)++;
		prof.at(index(im.c0, im.c1, dt))++;
	}

	// normalize bins
	for (int c0 = 0; c0 < cbins; c0++) {
	for (int dt = 0; dt < tbins; dt++) {
		int ci = c0*tbins + dt;
		if (cnt.at(ci) == 0)
			continue;

		int base = c0*tbins*cbins + dt*cbins;
		for (int c1 = 0; c1 < cbins; c1++)
			prof.at(base + c1) /= cnt[ci];
	}
	}
}

void AnyProf::maxes(const std::vector<Imp> &imps) {
	for (unsigned int i = 0; i < imps.size(); i++) {
		const struct Imp &im = imps[i];

		if (im.c0 > cmax)
			cmax = im.c0;
		assert (im.c1 <= im.c0);

		if (im.t1 > tmax)
			tmax = im.t1;

		double dt = im.t1 - im.t0;
		assert (dt > 0);
		if (dt > dtmax)
			dtmax = dt;
	}
}