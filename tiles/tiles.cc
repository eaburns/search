#include "tiles.hpp"
#include "../incl/utils.hpp"
#include <cstdlib>
#include <cerrno>

Tiles::Tiles(FILE *in) {
	readruml(in);
}

void Tiles::readruml(FILE *in) {
	unsigned int w, h;

	if (fscanf(in, " %u %u", &w, &h) != 2)
		fatalx(errno, "Failed to read width and height");

	if (w != Width && h != HEIGHT)
		fatal("Width and height instance/compiler option mismatch");

	if (fscanf(in, " starting positions for each tile:") != 0)
		fatalx(errno, "Failed to read the starting position label");

	for (Tile t = 0; t < Ntiles; t++) {
		unsigned int p;
		int r = fscanf(in, " %u", &p);
		if (r != 1)
			fatalx(errno, "Failed to read the starting positions: r=%d", r);
		init[p] = t;
	}

	if (fscanf(in, " goal positions:") != 0)
		fatalx(errno, "Failed to read the goal position label");

	for (Tile t = 0; t < Ntiles; t++) {
		unsigned int p;
		if (fscanf(in, " %u", &p) != 1)
			fatalx(errno, "Failed to read the goal positions");
		goalpos[t] = p;
	}
}

TilesMdist::TilesMdist(FILE *in) : Tiles(in) {
	initmd();
	initincr();
}

void TilesMdist::initmd(void) {
	for (int t = 1; t < Ntiles; t++) {
		unsigned int row = goalpos[t] / Width;
		unsigned int col = goalpos[t] % Width;
		for (int i = 0; i < Ntiles; i++) {
			unsigned int r = i / Width;
			unsigned int c = i % Width;
			md[t][i] = abs(r - row) + abs(c - col);
		}
	}
}

void TilesMdist::initincr(void) {
	for (int t = 1; t < Ntiles; t++) {
	for (int old = 0; old < Ntiles; old++) {
		unsigned int cur = md[t][old];
		for (int nw = 0; nw <Ntiles; nw++)
			incr[t][old][nw] = cur - md[t][nw];
	}
	}
}

Tiles::Cost TilesMdist::mdist(State *s) {
	unsigned int sum = 0;

	for (int i = 0; i < Ntiles; i++) {
		Tile t = s->ts[i];
		if (t == 0)
			continue;
		sum += md[t][i];
	}

	return sum;
}