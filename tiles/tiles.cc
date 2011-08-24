#include "tiles.hpp"
#include "../incl/utils.hpp"
#include <cstdlib>
#include <cerrno>
#include <cstdio>

Tiles::Tiles(FILE *in) {
	readruml(in);
	initops();
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

void Tiles::initops(void) {
	for (int i = 0; i < Ntiles; i++) {
		ops[i].n = 0;
		if (i >= Width)
			ops[i].mvs[ops[i].n++] = i - Width;
		if (i % Width > 0)
			ops[i].mvs[ops[i].n++] = i - 1;
		if (i % Width < Width - 1)
			ops[i].mvs[ops[i].n++] = i + 1;
		if (i < Ntiles - Width)
			ops[i].mvs[ops[i].n++] = i + Width;
	}
}

void Tiles::dumptiles(FILE *out, Tile ts[]) {
	for (int i = 0; i < Ntiles; i++) {
		if (i > 0 && i % Width == 0)
			fprintf(out, "\n");
		else if (i > 0)
			fprintf(out, "\t");
		fprintf(out, "%2d", ts[i]);
	}
	fprintf(out, "\n");
}

TilesMdist::TilesMdist(FILE *in) : Tiles(in) {
	initmd();
	initincr();
}

TilesMdist::State TilesMdist::initstate(void) {
	State s;
	s.h = 0;
	for (int i = 0; i < Ntiles; i++) {
		if (init[i] == 0)
			s.b = i;
		else
			s.h += md[init[i]][i];
		s.ts[i] = init[i];
	}
	return s;
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
		for (unsigned int n = 0; n <ops[old].n; n++) {
			unsigned int nw = ops[old].mvs[n];
			incr[t][old][nw] = md[t][nw] - cur;
		}
	}
	}
}