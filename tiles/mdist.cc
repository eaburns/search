#include "mdist.hpp"
#include <cstdlib>

TilesMdist::TilesMdist(FILE *in) : Tiles(in) {
	initmd();
	initincr();
}

TilesMdist::State TilesMdist::initialstate(void) {
	State s;
	s.h = 0;
	for (int i = 0; i < Ntiles; i++) {
		if (init[i] == 0)
			s.b = i;
		else
			s.h += md[init[i]][i];
		s.ts[i] = init[i];
	}
	return s;
}

void TilesMdist::initmd(void) {
	for (int t = 1; t < Ntiles; t++) {
		unsigned int row = goalpos[t] / Width;
		unsigned int col = goalpos[t] % Width;
		for (int i = 0; i < Ntiles; i++) {
			unsigned int r = i / Width;
			unsigned int c = i % Width;
			md[t][i] = abs(r - row) + abs(c - col);
		}
	}
}

void TilesMdist::initincr(void) {
	for (int t = 1; t < Ntiles; t++) {
	for (int old = 0; old < Ntiles; old++) {
		unsigned int cur = md[t][old];
		for (unsigned int n = 0; n <ops[old].n; n++) {
			unsigned int nw = ops[old].mvs[n];
			incr[t][old][nw] = md[t][nw] - cur;
		}
	}
	}
}