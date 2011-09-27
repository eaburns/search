#include "mdlearn.hpp"
#include <cstdlib>
#include <cstring>
#include <boost/array.hpp>
#include <algorithm>

TilesMDLearn::TilesMDLearn(FILE *in) : TilesMdist(in) {
	initops(10);
}

void TilesMDLearn::initops(unsigned int dmax) {
	unsigned int oldsz = ops.size();
	if (dmax+1 > ops.capacity())
		ops.reserve((dmax+1) * 2);
	ops.resize(dmax+1);

	for (unsigned int d = 0; d < oldsz; d++)
		for (unsigned int i = 0; i < Ntiles; i++)
			ops[d][i].initdests();

	for (unsigned int d = oldsz; d <= dmax; d++) {
		for (unsigned int i = 0; i < Ntiles; i++)
			ops[d][i].init(i);
	}
}

void TilesMDLearn::Opvec::init(Pos i) {
	if (i >= Width)
		mvs[n++].b = i - Width;
	if (i % Width > 0)
		mvs[n++].b = i - 1;
	if (i % Width < Width - 1)
		mvs[n++].b = i + 1;
	if (i < Ntiles - Width)
		mvs[n++].b = i + Width;
	initdests();
}
