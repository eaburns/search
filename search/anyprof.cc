#include "anyprof.hpp"
#include "../utils/utils.hpp"
#include <cmath>
#include <cstring>
#include <limits>

AnyProf::AnyProf(unsigned int cbs, double cm, unsigned int tbs, double tm) :
		cbins(cbs), tbins(tbs), cmax(cm), tmax(tm),
		cwidth(cmax/(cbins-1)), twidth(tmax/(tbins-1)) {

	assert (floor(cmax/cwidth) == cbins-1);
	assert (floor(tmax/twidth) == tbins-1);

	bins.resize(cbins);
	for (unsigned int i = 0; i < cbins; i++) {
		bins[i].resize(tbins);
		for (unsigned int j = 0; j < cbins; j++)
			bins[i][j].resize(cbins, 0);
	}
}