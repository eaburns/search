// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "plat2d.hpp"
#include <cstdio>

int main() {
	Lvl lvl(stdin);
	bool *blkd = new bool[lvl.width() * lvl.height()];
	for (unsigned int i = 0; i < lvl.width() * lvl.height(); i++)
		blkd[i] = false;
	for (unsigned int i = 0; i < lvl.width(); i++) {
	for (unsigned int j = 0; j < lvl.height(); j++)
		blkd[i * lvl.height() + j] = lvl.blocked(i, lvl.height() - j - 1);
	}

	PolyMap polys(blkd, lvl.width(), lvl.height());
	polys.output(stdout);
	return 0;
}