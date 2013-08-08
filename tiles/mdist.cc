// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "mdist.hpp"
#include <cstdlib>
#include <limits>
#include <vector>

TilesMdist::TilesMdist(FILE *in) : Tiles(in) {
	initmd();
	initincr();
}

TilesMdist::State TilesMdist::initialstate() {
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

void TilesMdist::initmd() {
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

void TilesMdist::initincr() {
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

TilesMdist::Cost TilesMdist::pathcost(const std::vector<State> &path, const std::vector<Oper> &ops) {
	State state = initialstate();
	Cost cost(0);

	if (ops.size() > (unsigned int) std::numeric_limits<int>::max())
		fatal("Too many actions");

	for (int i = ops.size() - 1; i >= 0; i--) {
		State copy(state);
		Edge e(*this, copy, ops[i]);
		cost += e.cost;
		assert(e.state == path[i]);
		state = e.state;
	}
	assert (isgoal(state));
	return cost;
}