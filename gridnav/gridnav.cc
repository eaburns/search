#include "gridnav.hpp"
#include <cstdio>
#include <limits>

GridNav::GridNav(GridMap *m, unsigned int x0, unsigned int y0,
		unsigned int x1, unsigned int y1) : map(m) {
	start = map->loc(x0, y0);
	finish = map->loc(x1, y1);
}

GridNav::State GridNav::initialstate(void) {
	State s;
	s.loc = start;
	return s;
}