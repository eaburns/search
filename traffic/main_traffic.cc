#include "traffic.hpp"
#include "../search/main.hpp"
#include "../search/fhatident.hpp"
#include <cstdio>
#include <cerrno>

static SearchAlgorithm<Traffic> *get(int, const char*[]);

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

	GridMap map(lvl);

	unsigned int x0, y0, xg, yg;
	if (fscanf(stdin, "%u %u %u %u\n", &x0, &y0, &xg, &yg) != 4)
		fatal("Failed to read start and end locations");

	if (lvlpath[0] != '\0') {
		dfpair(stdout, "level", "%s", lvlpath);
		fclose(lvl);
 	}

	dfpair(stdout, "start x", "%u", x0);
	dfpair(stdout, "start y", "%u", y0);
	dfpair(stdout, "goal x", "%u", xg);
	dfpair(stdout, "goal y", "%u", yg);

	Traffic d(&map, x0, y0, xg, yg);
	searchGet<Traffic>(get, d, argc, argv);

	dffooter(stdout);

	return 0;
}

static SearchAlgorithm<Traffic> *get(int argc, const char *argv[]) {
	if (strcmp(argv[1], "fhatident") == 0)
		return new Fhatident<Traffic>(argc, argv);
	return getsearch<Traffic>(argc, argv);
}
