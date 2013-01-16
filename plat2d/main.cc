//#include "closedlist.hpp"
#include "plat2d.hpp"
#include "../search/main.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cerrno>

static const char *lvl, *vg;

static void parseargs(int, const char*[]);

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

	FILE *vgfile = NULL;
	if (vg) {
		vgfile = fopen(vg, "r");
		if (!vgfile)
			fatalx(errno, "Failed to open %s for reading", vg);
		dfpair(stdout, "visibility graph loaded from", "%s", vg);
	}

	Plat2d d(infile, vgfile);
	if (infile != stdin)
		fclose(infile);

	search<Plat2d>(d, argc, argv);
	dffooter(stdout);

	return 0;
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (i < argc - 1 && strcmp(argv[i], "-lvl") == 0)
			lvl = argv[++i];
		else if (i < argc - 1 && strcmp(argv[i], "-vg") == 0)
			vg = argv[++i];
	}
}
