// randinst makes a random instance for a give map.
#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include <cstring>
#include <cerrno>

static void helpmsg(int);

int main(int argc, char *argv[]) {
	std::string mappath;
	unsigned long seed = walltime();
	for (int i = 1; i < argc; i++) {
		if (i < argc-1 && strcmp(argv[i], "-m") == 0) {
			mappath = argv[++i];
		} else if (i < argc-1 && strcmp(argv[i], "-s") == 0) {
			char *end = NULL;
			seed = strtoul(argv[++i], &end, 10);
			if (end == argv[i])
				fatal("invalid seed: %s", argv[i]);
		} else if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);
		} else {
			helpmsg(1);
		}
	}
	if (mappath == "")
		helpmsg(1);

	GridMap map(mappath);
	Rand r(seed);

	// Get start and goal locations as any pair
	// of unblocked locations.
	unsigned int xs[2];
	unsigned int ys[2];
	for (int i = 0; i < 2; i++) {
again:
		xs[i] = r.integer(1, map.w);
		ys[i] = r.integer(1, map.h);
		if (map.blkd(map.index(xs[i], ys[i])))
			goto again;
	}

	// Dump the map to standard output followed
	// by the the points.
	FILE *in = fopen(mappath.c_str(), "r");
	if (!in)
		fatalx(errno, "failed to open %s for reading", mappath.c_str());
	while (!feof(in) && !ferror(in)) {
		char buf[1024];
		size_t n = fread(buf, 1, sizeof(buf), in);
		while (n > 0) {
			size_t m = fwrite(buf, 1, n, stdout);
			n -= m;
		}
	}
	if (ferror(in))
		fatal("error while reading %s", mappath.c_str());
	fclose(in);
	printf("%u %u %u %u\n", xs[0]-1, ys[0]-1, xs[1]-1, ys[1]-1);

	return 0;
}



static void helpmsg(int status) {
	puts("usage: randinst [-h] [-s <seed>] -m <map>");
	exit(status);
}