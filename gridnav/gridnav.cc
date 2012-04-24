#include "gridnav.hpp"
#include <cstdio>
#include <limits>
#include <cassert>

GridNav::GridNav(GridMap *m, unsigned int x0, unsigned int y0,
		unsigned int x1, unsigned int y1) : map(m) {
	start = map->index(x0+1, y0+1);
	finish = map->index(x1+1, y1+1);
	reverseops();
}

GridNav::State GridNav::initialstate(void) const {
	State s;
	s.loc = start;
	return s;
}

void GridNav::reverseops(void) {
	unsigned int nrev = 0;
	for (unsigned int i = 0; i < map->nmvs; i++) {
	for (unsigned int j = 0; j < map->nmvs; j++) {
		if (map->mvs[i].dx != -map->mvs[j].dx || map->mvs[i].dy != -map->mvs[j].dy)
			continue;
		rev[i] = j;
		nrev++;
		break;
	} 
	}
	assert (nrev == map->nmvs);
}

GridNav::Cost GridNav::pathcost(const std::vector<Oper> &ops) const {
	GridNav::State state = initialstate();
	GridNav::Cost cost(0);
	for (int i = ops.size() - 1; i >= 0; i--) {
		GridNav::State copy(state);
		GridNav::Edge e(*this, copy, ops[i]);
		state = e.state;
		cost += e.cost;
	}
	assert (isgoal(state));
	return cost;
}