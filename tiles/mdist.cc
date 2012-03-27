#include "mdist.hpp"
#include <cstdlib>

TilesMdist::TilesMdist(FILE *in) : Tiles(in) {
	initmd();
	initincr();
}

TilesMdist::State TilesMdist::initialstate(void) {
	State s;
	s.h = 0;
	for (unsigned int i = 0; i < Ntiles; i++) {
		if (init[i] == 0)
			s.b = i;
		else
			s.h += md[init[i]][i];
		s.ts[i] = init[i];
	}
	return s;
}

void TilesMdist::initmd(void) {
	for (unsigned int t = 1; t < Ntiles; t++) {
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
	for (unsigned int t = 1; t < Ntiles; t++) {
	for (unsigned int nw = 0; nw < Ntiles; nw++) {
		unsigned int next = md[t][nw];
		for (unsigned int n = 0; n <ops[nw].n; n++) {
			unsigned int old = ops[nw].mvs[n];
			incr[t][nw][old] = md[t][old] - next;
		}
	}
	}
}