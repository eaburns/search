#include "gridmap.hpp"
#include "../visnav/polymap.hpp"
#include <cstdio>

int main(int argc, char *argv[]) {
	GridMap map(stdin);
	unsigned int sz = (map.width() + 2) * (map.height() + 2);
	unsigned int stride = map.height() + 2;
	bool *blkd = new bool[sz];
	for (unsigned int i = 0; i < sz; i++)
		blkd[i] = true;
	for (unsigned int i = 0; i < map.width(); i++) {
		unsigned int base = (i + 1) * stride;

		for (unsigned int j = 0; j < map.height(); j++)
			blkd[base + (j + 1)] = !map.passable(map.loc(i, j));
	}

	PolyMap polys(blkd, map.width() + 2, map.height() + 2);
	polys.output(stdout);

	return 0;
}