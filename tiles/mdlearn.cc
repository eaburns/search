#include "mdlearn.hpp"
#include <cstdlib>
#include <cstring>
#include <boost/array.hpp>
#include <algorithm>

TilesMDLearn::TilesMDLearn(FILE *in) : TilesMdist(in) {
	resize(100);
}

void TilesMDLearn::resize(unsigned int dmax) {
	unsigned int oldsz = ops.size();
	if (dmax+1 > ops.capacity())
		ops.reserve((dmax+1) * 2);
	ops.resize(dmax+1);

	for (unsigned int d = oldsz; d <= dmax; d++) {
		ops[d] = new Opvec[Ntiles];
		for (unsigned int i = 0; i < Ntiles; i++)
			ops[d][i].init(i);
	}
}

void TilesMDLearn::Opvec::init(Pos i) {
	n = 0;
	if (i >= Width) {
		mvs[n++].b = i - Width;
		sorted[n-1].ptr = &mvs[n-1];
		dests[mvs[n-1].b] = &mvs[n-1];
	}
	if (i % Width > 0) {
		mvs[n++].b = i - 1;
		sorted[n-1].ptr = &mvs[n-1];
		dests[mvs[n-1].b] = &mvs[n-1];
	}
	if (i % Width < Width - 1) {
		mvs[n++].b = i + 1;
		sorted[n-1].ptr = &mvs[n-1];
		dests[mvs[n-1].b] = &mvs[n-1];
	}
	if (i < Ntiles - Width) {
		mvs[n++].b = i + Width;
		sorted[n-1].ptr = &mvs[n-1];
		dests[mvs[n-1].b] = &mvs[n-1];
	}
}
