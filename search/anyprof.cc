#include "anyprof.hpp"
#include <cmath>
#include <cstring>

AnyProf::AnyProf(const std::vector<Imp> &imps, int cb, int tb) :
		cmax(0), tmax(0), cbins(cb), tbins(tb), sz(cb*cb*tb) {
	maxes(imps);
	cwidth = ceil(cmax) / cbins;
	twidth = ceil(tmax) / tbins;

	prof = new double[sz];
	for (int i = 0; i < sz; i++)
		prof[i] = 0;

	int *cnt = new int[cbins*tbins];
	memset(cnt, 0, sizeof(*cnt) * cbins * tbins);

	// counts for each bin
	for (unsigned int i = 0; i < imps.size(); i++) {
		const struct Imp &im = imps[i];
		assert (im.c0/cwidth < cbins);
		assert (im.dt/twidth < tbins);
		int ci = (im.c0/cwidth)*tbins + im.dt/twidth;
		assert (ci < cbins*tbins);
		assert (ci >= 0);
		cnt[ci]++;
		prof[index(im.c0, im.c1, im.dt)]++;
	}

	// normalize bins
	for (int c0 = 0; c0 < cbins; c0++) {
	for (int dt = 0; dt < tbins; dt++) {
		int ci = c0*tbins + dt;
		assert (ci < cbins*tbins);
		assert (ci >= 0);
		if (cnt[ci] == 0)
			continue;

		int base = c0*tbins*cbins + dt*cbins;
		for (int c1 = 0; c1 < cbins; c1++) {
			assert (base + c1 < sz);
			assert (base + c1 >= 0);
			prof[base + c1] /= cnt[ci];
		};
	}
	}

	delete[] cnt;
}

AnyProf::~AnyProf(void) {
	if (prof)
		delete[] prof;
}

void AnyProf::maxes(const std::vector<Imp> &imps) {
	for (unsigned int i = 0; i < imps.size(); i++) {
		const struct Imp &im = imps[i];
		if (im.c0 > cmax)
			cmax = im.c0;
		assert (im.c1 <= im.c0);
		if (im.dt > tmax)
			tmax = im.dt;
	}
}