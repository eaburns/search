// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "gridmap.hpp"
#include "gridnav.hpp"
#include "../utils/utils.hpp"
#include "../search/greedy.hpp"
#include <cstring>

static bool lifecost, eightway;
static unsigned int width, height;
static double prob;
uint64_t seed = walltime();

// start and goal locations.
unsigned int xs, ys, xg, yg;

static bool solvable(GridMap&);
static void parseargs(int, char*[]);
static void helpmsg(int);

int main(int argc, char *argv[]) {
	parseargs(argc, argv);


	xs = 0;
	ys = 0;
	xg = width-1;
	yg = 0;

	unsigned int i = 0;
	for ( ; ; ) {
		GridMap m = GridMap::uniform(width, height, prob, seed+i);
		if (eightway)
			m.seteightway();

		if (!m.blkd(m.index(xs+1, ys+1)) && !m.blkd(m.index(xg+1, yg+1)) && solvable(m))
			break;

		i++;
		putc('.', stderr);
	}
	if (i > 0)
		putc('\n', stderr);

	printf("seed %lu %u %u %f\n", (unsigned long) seed+i,
		width, height, prob);
	if (eightway)
		puts("Eightway");
	else
		puts("Fourway");
	if (lifecost)
		puts("Life");
	else
		puts("Unit");
	printf("%u %u\n", xs, ys);
	printf("%u %u\n", xg, yg);

	return 0;
}

static bool solvable(GridMap &m) {
	GridNav d(&m, xs, ys, xg, yg);
	Greedy<GridNav, true> speedy(0, NULL);
	GridNav::State s0 = d.initialstate();
	speedy.search(d, s0);
	return speedy.res.path.size() > 0;
}

static void parseargs(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);

		} else if (i < argc-1 && strcmp(argv[i], "-seed") == 0) {
			seed = strtoull(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-width") == 0) {
			width = strtoul(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-height") == 0) {
			height = strtoul(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-prob") == 0) {
			prob = strtod(argv[++i], NULL);

		} else if (strcmp(argv[i], "-lifecost") == 0) {
			lifecost = true;

		} else if (strcmp(argv[i], "-eightway") == 0) {
			eightway = true;

		} else {
			helpmsg(1);
		}
	}

	if (prob == 0)
		fatal("prob must be positive");
	if (width == 0)
		fatal("width must be positive");
	if (height == 0)
		fatal("height must be positive");
}

static void helpmsg(int status) {
	puts("usage: mkseedinst [options]");
	puts("-h	print this help message");
	puts("-width <number>	specify the width");
	puts("-height <number>	specify the height");
	puts("-prob <probability>	specify obstacle probability");
	puts("-lifecost	use life cost");
	puts("-eightway	use eight-way movement");
	exit(status);
}
