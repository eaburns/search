#include "visnav.hpp"
#include "../search/main.hpp"
#include <cstdio>
#include <cerrno>

int main(int argc, const char *argv[]) {
	VisGraph g(stdin);
	double x0, y0, x1, y1;
	if (fscanf(stdin, " %lg %lg %lg %lg", &x0, &y0, &x1, &y1) != 4)
		fatal("Failed to read start and finish locations");

	dfpair(stdout, "x0", "%g", x0);
	dfpair(stdout, "y0", "%g", y0);
	dfpair(stdout, "x1", "%g", x1);
	dfpair(stdout, "y1", "%g", y1);
	VisNav d(g, x0, y0, x1, y1);
	Result<VisNav> res = search<VisNav>(d, argc, argv);
	d.save("path.eps", res.path);

	return 0;
}