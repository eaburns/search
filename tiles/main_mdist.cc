#include "mdist.hpp"
#include "../search/main.hpp"
#include <cstdio>

static SearchAlgorithm<TilesMdist> *get(int, const char *[]);

int main(int argc, const char *argv[]) {
	dfheader(stdout);

	FILE *lvl = stdin;
	const char *lvlpath = "";
	for (int i = 0; i < argc; i++) {
		if (i < argc - 1 && strcmp(argv[i], "-lvl") == 0)
			lvlpath = argv[++i];
	}

	if (lvlpath[0] != '\0') {
		lvl = fopen(lvlpath, "r");
		if (!lvl)
			fatalx(errno, "Failed to open %s for reading", lvlpath);
	}

	TilesMdist d(lvl);

	if (lvlpath[0] != '\0') {
		dfpair(stdout, "level", "%s", lvlpath);
		fclose(lvl);
 	}

	searchGet<TilesMdist>(get, d, argc, argv);
	dffooter(stdout);
	return 0;
}

static SearchAlgorithm<TilesMdist> *get(int argc, const char *argv[]) {
	return getsearch<TilesMdist>(argc, argv);
}