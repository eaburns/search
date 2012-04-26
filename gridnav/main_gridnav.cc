#include "gridnav.hpp"
#include "closedlist.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	GridMap map(stdin);

	unsigned int x0, y0, xg, yg;
	if (fscanf(stdin, " %u %u %u %u", &x0, &y0, &xg, &yg) != 4)
		fatal("Failed to read start and end locations");

	GridNav d(&map, x0, y0, xg, yg);
	search<GridNav>(d, argc, argv);

	return 0;
}