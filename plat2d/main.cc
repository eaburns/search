#include "plat2d.hpp"
#include "../search/main.hpp"
#include "../search/rrt.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cerrno>
#include <google/profiler.h>

const char *lvl = NULL;

static void parseargs(int, const char*[]);
static SearchAlgorithm<Plat2d> *get(int, const char*[]);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);
	dfheader(stdout);

	FILE *infile = stdin;
	if (lvl) {
		infile = fopen(lvl, "r");
		if (!infile)
			fatalx(errno, "Failed to open %s for reading", lvl);
		dfpair(stdout, "level", "%s", lvl);
	}

	Plat2d d(infile);
	if (infile != stdin)
		fclose(infile);

	ProfilerStart("cpu.prof");
	searchGet<Plat2d>(get, d, argc, argv);
	ProfilerStop();
	dffooter(stdout);

	return 0;
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (i < argc - 1 && strcmp(argv[i], "-lvl") == 0)
			lvl = argv[++i];
	}
}

static SearchAlgorithm<Plat2d> *get(int argc, const char *argv[]) {
	if (strcmp(argv[1], "rrt") == 0)
		return new RRT<Plat2d>(argc, argv);
	return getsearch<Plat2d>(argc, argv);
}