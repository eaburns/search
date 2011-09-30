#include "gridpath.hpp"
#include "../search/main.hpp"
#include "../utils/utils.hpp"
#include <iostream>
#include <string>
#include <cmath>

const float Epsilon = 0.0001;

int main(int argc, char *argv[]) {
	float ver;
	std::string verstr;

	std::cin >> verstr >> ver;
	if (verstr != "version")
		fatal("Expected a version header");

	if (ver != 1.0)
		fatal("Version %g is unsupported.  Latest supported version is 1.0", ver);

	unsigned int lineno = 1;
	while (!feof(stdin) && !ferror(stdin)) {
		std::string map;
		unsigned int bucket;
		unsigned int w, h;
		unsigned int x0, y0;
		unsigned int x1, y1;
		float opt;

		std::cin >> bucket >> map >> w >> h >> x0 >> y0 >> x1 >> y1 >> opt;
		lineno++;

		GridPath d(map.c_str(), x0, y0, x1, y1);
		if (d.width() != w || d.height() != h)
			fatal("Map dimensions don't match the scenario dimensions");

		Result<GridPath> r = search<GridPath>(d, argc, argv);
		dfpair(stdout, "bucket", "%u", bucket);
		dfpair(stdout, "map width", "%u", w);
		dfpair(stdout, "map height", "%u", h);
		dfpair(stdout, "start x", "%u", x0);
		dfpair(stdout, "start y", "%u", y0);
		dfpair(stdout, "finish x", "%u", x1);
		dfpair(stdout, "finish y", "%u", y1);
		
		if (fabsf(r.cost - opt) > Epsilon)
			fatal("Scenario line %d: expected optimal cost %g, got %g\n",
				lineno, opt, r.cost);

		putc('\n', stdout);
	}

	if (ferror(stdin))
		fatal("Error reading scenario");

	return 0;
}