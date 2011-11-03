#include "closedlist.hpp"
#include "gridpath.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, char *argv[]) {
	GridMap map(stdin);

	if (map.movetype() != GridMap::FourWay && map.movetype() != GridMap::EightWay)
		fatal("Expected a Ruml instance");

	unsigned int x0, y0, xg, yg;
	if (fscanf(stdin, " %u %u %u %u", &x0, &y0, &xg, &yg) != 4)
		fatal("Failed to read start and end locations");

	GridPath d(&map, x0, y0, xg, yg);
	search<GridPath>(d, argc, argv);

	return 0;
}