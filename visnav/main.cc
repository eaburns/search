#include "visnav.hpp"
#include "../search/main.hpp"
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
	std::string file;
	double x0, y0, x1, y1;
	std::cin >> file >> x0 >> y0 >> x1 >> y1;

	FILE *f = fopen(file.c_str(), "r");
	if (!f)
		fatalx(errno, "Failed to open %s for reading", file.c_str());
	VisGraph g(f);
	fclose(f);

	dfpair(stdout, "x0", "%g", x0);
	dfpair(stdout, "y0", "%g", y0);
	dfpair(stdout, "x1", "%g", x1);
	dfpair(stdout, "y1", "%g", y1);
	VisNav nav(g, x0, y0, x1, y1);
	Result<VisNav> res = search<VisNav>(nav, argc, argv);

	nav.save("path.eps", res.path);

	return 0;
}