// Read an graph on stdin, place two points
// in the unit square that are not within a
// polygon and output the graph along
// with the two points.

#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cerrno>

int main(int argc, char *argv[]) {
	FILE *f = stdin;
	if (argc >= 2) {
		f = fopen(argv[1], "r");
		if (!f)
			fatalx(errno, "Failed to open %s for reading", argv[1]);
	}

	VisGraph g(f);

	if (argc >= 2)
		fclose(f);

	double x0, y0;
	do {
		x0 = randgen.real();
		y0 = randgen.real();
	} while (g.obstructed(Geom2d::Pt(x0, y0)));

	double x1, y1;
	do {
		x1 = randgen.real();
		y1 = randgen.real();
	} while (g.obstructed(Geom2d::Pt(x1, y1)));

	g.output(stdout);
	printf("%g %g %g %g\n", x0, y0, x1, y1);
}