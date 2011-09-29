#include "gridpath.hpp"
#include <cstdio>
#include <limits>

GridPath::GridPath(FILE *in) : map(in) {
	start = map.width() + 1;
	finish = (map.width() - 1) * map.height() - 2;
	printf("Starting at %u, %u\n", map.x(start), map.y(start));
	printf("Ending at %u, %u\n", map.x(finish), map.y(finish));
	map.output(stdout);
}

GridPath::State GridPath::initialstate(void) {
	State s;
	s.loc = start;
	return s;
}