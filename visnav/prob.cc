#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cerrno>

int main(int argc, char *argv[]) {
	if (argc < 2)
		fatal("Usage: prob <graph file>");

	FILE *f = fopen(argv[1], "r");
	if (!f)
		fatalx(errno, "Failed to open %s for reading", argv[1]);
	VisGraph g(f);
	fclose(f);

	double x0, y0;
	do {
		x0 = randgen.real();
		y0 = randgen.real();
	} while (!g.isoutside(x0, y0));

	double x1, y1;
	do {
		x1 = randgen.real();
		y1 = randgen.real();
	} while (!g.isoutside(x1, y1));

	printf("%s %g %g %g %g\n", argv[1], x0, y0, x1, y1);
}