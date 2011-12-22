#include "plat2d.hpp"
#include "../visnav/visgraph.hpp"
#include <vector>
#include <cstdio>

int main(void) {
	Lvl lvl(stdin);
	bool *blkd = new bool[lvl.width() * lvl.height()];
	for (unsigned int i = 0; i < lvl.width() * lvl.height(); i++)
		blkd[i] = false;
	for (unsigned int i = 0; i < lvl.width(); i++) {
	for (unsigned int j = 0; j < lvl.height(); j++)
		blkd[i * lvl.height() + j] = lvl.blocked(i, lvl.height() - j - 1);
	}

	VisGraph gv(blkd, lvl.width(), lvl.height());
	gv.output(stdout);
	return 0;
}