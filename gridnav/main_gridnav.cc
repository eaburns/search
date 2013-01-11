#include "gridnav.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	dfheader(stdout);
	GridMap map(stdin);

	unsigned int x0, y0, xg, yg;
	if (fscanf(stdin, " %u %u %u %u", &x0, &y0, &xg, &yg) != 4)
		fatal("Failed to read start and end locations");

	dfpair(stdout, "start x", "%u", x0);
	dfpair(stdout, "start y", "%u", y0);
	dfpair(stdout, "goal x", "%u", xg);
	dfpair(stdout, "goal y", "%u", yg);

	if (map.blkd(map.index(x0+1, y0+1)))
		fatal("The start location is blocked");
	if (map.blkd(map.index(xg+1, yg+1)))
		fatal("The goal location is blocked");

	GridNav d(&map, x0, y0, xg, yg);
	search<GridNav>(d, argc, argv);
	dffooter(stdout);

	return 0;
}