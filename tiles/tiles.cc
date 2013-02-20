#include "tiles.hpp"
#include "../utils/utils.hpp"
#include "../utils/safeops.hpp"
#include <cerrno>
#include <cstdio>
#include <ctime>
#include <limits>

bool Tiles::hashvecinit = false;
unsigned long Tiles::hashvec[Ntiles][Ntiles];

Tiles::Tiles() {
	initops();
	if (!hashvecinit)
		inithashvec();
}

Tiles::Tiles(FILE *in) {
	readruml(in);
	initops();
	if (!hashvecinit)
		inithashvec();
}

void Tiles::readruml(FILE *in) {
	unsigned int w, h;

	if (fscanf(in, " %u %u", &w, &h) != 2)
		fatal("Failed to read width and height");

	if (!safe::can_mul(w, h))
		fatal("The tiles board is too big");

	if (w != Width && h != HEIGHT)
		fatal("Width and height instance/compiler option mismatch");

	if (fscanf(in, " starting positions for each tile:") != 0)
		fatal("Failed to read the starting position label");

	for (Tile t = 0; t < Ntiles; t++) {
		unsigned int p;
		int r = fscanf(in, " %u", &p);
		if (r != 1)
			fatal("Failed to read the starting position for tile %d", t);
		init[p] = t;
	}

	if (fscanf(in, " goal positions:") != 0)
		fatal("Failed to read the goal position label");

	for (Tile t = 0; t < Ntiles; t++) {
		unsigned int p;
		if (fscanf(in, " %u", &p) != 1)
			fatal("Failed to read the goal position");
		goalpos[t] = p;
	}
}

void Tiles::initops() {
	for (unsigned int i = 0; i < Ntiles; i++) {
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

void Tiles::inithashvec() {
	hashvecinit = true;
	Rand r(time(NULL));
	for (int i = 0; i < Ntiles; i++) {
	for (int j = 0; j < Ntiles; j++)
		hashvec[i][j] = r.bits();
	}
}

void Tiles::dumptiles(FILE *out, Tile ts[]) {
	for (unsigned int i = 0; i < Ntiles; i++) {
		if (i > 0 && i % Width == 0)
			fprintf(out, "\n");
		else if (i > 0)
			fprintf(out, "\t");
		fprintf(out, "%2d", ts[i]);
	}
	fprintf(out, "\n");
}