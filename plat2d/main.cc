#include "closedlist.hpp"
#include "../search/main.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cerrno>

const char *lvl = NULL;

static void parseargs(int, const char*[]);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);

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

	Result<Plat2d> res = search<Plat2d>(d, argc, argv);

	std::vector<unsigned int> controls;
	for (int i = res.path.size() - 1; i > 0; i--) {
		if (i < (int) res.path.size() - 1)
			controls.push_back(res.path[i].control);
	}
	dfpair(stdout, "controls", "%s", controlstr(controls).c_str());

	return 0;
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (i < argc - 1 && strcmp(argv[i], "-lvl") == 0)
			lvl = argv[++i];
	}
}
