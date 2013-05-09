#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include "../utils/safeops.hpp"
#include <cstdio>
#include <cstddef>
#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <string>

void GridMap::load(FILE *in) {
	if (fscanf(in, "%u %u\n", &w, &h) != 2)
		fatal("Failed to read map header");

	sz = w * h;

	int x, y, dx, dy;
	while(fscanf(in, "%d %d %d %d\n", &x, &y, &dx, &dy) == 4) {
		obstacles.emplace_back(x,y,dx,dy);
	}
	setfourway();
}

GridMap::~GridMap() {}


GridMap::Move::Move(const GridMap &m, const char *nme, int deltax, int deltay, unsigned int num, ...) :
			dx(deltax), dy(deltay), delta(dx + m.w * dy), cost(1.0), n(num + 1), name(nme) {
	if (n > sizeof(chk) / sizeof(chk[0]))
		fatal("Cannot create a move with %d checks\n", n);

	if (dx != 0 && dy != 0)
		cost = sqrt(2.0);

	va_list ap;
	va_start(ap, num);
	chk[0].dx = dx;
	chk[0].dy = dy;
	chk[0].delta = delta;
	for (unsigned int i = 1; i < n; i++) {
		chk[i].dx = va_arg(ap, int);
		chk[i].dy = va_arg(ap, int);
		chk[i].delta = chk[i].dx + m.w * chk[i].dy;
	}
	va_end(ap);
}

// void GridMap::setoctile() {
// 	// This operator ordering seems to give more accurate
// 	// path-costs (compared to Sathan's scenario costs)
// 	// when simply using doubles as the cost type.
// 	nmvs = 0;
// 	mvs[nmvs++] = Move(*this, "W", -1,0, 0);
// 	mvs[nmvs++] = Move(*this, "E", 1,0, 0);
// 	mvs[nmvs++] = Move(*this, "N", 0,-1, 0);
// 	mvs[nmvs++] = Move(*this, "NW", -1,-1, 2, 0,-1, -1,0);
// 	mvs[nmvs++] = Move(*this, "NE", 1,-1, 2, 0,-1, 1,0);
// 	mvs[nmvs++] = Move(*this, "S", 0,1, 0);
// 	mvs[nmvs++] = Move(*this, "SW", -1,1, 2, -1,0, 0,1);
// 	mvs[nmvs++] = Move(*this, "SE", 1,1, 2, 1,0, 0,1);
// }

// void GridMap::seteightway() {
// 	nmvs = 0;
// 	mvs[nmvs++] = Move(*this, "SE", 1,1, 0);
// 	mvs[nmvs++] = Move(*this, "NE", 1,-1, 0);
// 	mvs[nmvs++] = Move(*this, "SW", -1,1, 0);
// 	mvs[nmvs++] = Move(*this, "NW", -1,-1, 0);
// 	mvs[nmvs++] = Move(*this, "S", 0,1, 0);
// 	mvs[nmvs++] = Move(*this, "N", 0,-1, 0);
// 	mvs[nmvs++] = Move(*this, "W", -1,0, 0);
// 	mvs[nmvs++] = Move(*this, "E", 1,0, 0);
// }

void GridMap::setfourway() {
	nmvs = 0;
	mvs[nmvs++] = Move(*this, "S", 0,1, 0);
	mvs[nmvs++] = Move(*this, "N", 0,-1, 0);
	mvs[nmvs++] = Move(*this, "W", -1,0, 0);
	mvs[nmvs++] = Move(*this, "E", 1,0, 0);
	mvs[nmvs++] = Move(*this, "NOP", 0,0, 0);
}
