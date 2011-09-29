#include "gridpath.hpp"
#include "../search/main.hpp"
#include "../utils/utils.hpp"
#include <cstdio>

int main(int argc, char *argv[]) {
	float ver;
	if (scanf("version %f", &ver) != 1)
		fatal("Failed to read version header");

	if (ver != 1.0)
		fatal("Version %g is unsupported.  Latest supported version is 1.0", ver);

	while (!feof(stdin) && !ferror(stdin)) {
		char map[1024];
		unsigned int bucket;
		unsigned int w, h;
		unsigned int x0, y0;
		unsigned int x1, y1;
		float len;

		if (scanf("%u", &bucket) != 1)
			break;
		if (scanf("%s", map) != 1)	// Dangerous1
			break;
		if (scanf(" %u %u %u %u %u %u %f", &w, &h, &x0, &y0,
			&x1, &y1, &len) != 7)
			break;

		GridPath d(map, x0, y0, x1, y1);
		if (d.width() != w || d.height() != h)
			fatal("Map dimensions don't match the scenario dimensions");

		search<GridPath>(d, argc, argv);
	}

	if (ferror(stdin))
		fatal("Error reading scenario");

	return 0;
}