#include "../utils/utils.hpp"
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdio>

using namespace std;

enum { BUFSZ = 100 };

static Rand *r;

static long unsigned int getrand(ptrdiff_t i) {
	return r->bits() % i;
}

void dumptiles(FILE *out, vector<int> ts, unsigned int width, unsigned int height) {
	for (unsigned int i = 0; i < width*height; i++) {
		if (i > 0 && i % width == 0)
			fprintf(out, "\n");
		else if (i > 0)
			fprintf(out, "\t");
		fprintf(out, "%2d", ts[i]);
	}
	fprintf(out, "\n");
}

static int solvable(vector<int> ts, unsigned int width, unsigned int height) {
	unsigned int parity = 0;
	unsigned int paritycheck = 2;
	unsigned int blankrow = 0;

	// Add parity bits through puzzle
	for (unsigned int i=0; i<ts.size(); i++) {
		if (ts[i] == 0)
			blankrow = i / (height) + 1;

		for (unsigned int j=0; j<i; j++) {
			if (ts[j] > ts[i] && ts[i] != 0)
				parity += 1;
		}
	}

	// add parity bit if grid is even width & other bullshit, see:
	// http://www.cs.bham.ac.uk/~mdr/teaching/modules04/java2/TilesSolvability.html
	if (width % 2 == 0) {
		if ((height-blankrow) % 2 == 0)
			parity += 1;
	}

	return parity%paritycheck;
}

void writeruml(vector<int> ts, int w, int h, char *dir, int n) {
	unsigned int Ntiles = ts.size();
	int output[Ntiles];
	char fname[BUFSZ];
	FILE *file = NULL;


	sprintf(fname, "%s/%d", dir, n);
	
	file = fopen(fname, "w");

	if (!file) {
		fatalx(errno, "Failed to open %s for writing", fname);
	}

	for (unsigned int i=0; i<Ntiles; i++) {
		output[ts[i]] = i;
	}

	fprintf(file, "%d %d\n", w, h);
	fprintf(file, "starting positions for each tile:\n");
	for (unsigned int i = 0; i<Ntiles; i++) {
		fprintf(file, "%d\n", output[i]);
	}
	fprintf(file, "goal positions:\n");
	for (unsigned int i = 0; i<Ntiles; i++)
		fprintf(file, "%d\n", i);
}

int main(int argc, char *argv[]) {
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int n = 1;
	unsigned int unsolvable = 0;
	long int seedval = -1;
	char *dir = NULL;
	vector<int> puzzle;
	
	for (int i=0; i<argc; i++) {
		if (strcmp(argv[i], "-w") == 0 && argc > i)
			width = atoi(argv[++i]);
		if (strcmp(argv[i], "-h") == 0 && argc > i)
			height = atoi(argv[++i]);
		if (strcmp(argv[i], "-n") == 0 && argc > i)
			n = atoi(argv[++i]);
		if (strcmp(argv[i], "-d") == 0 && argc > i)
			dir = argv[++i];
		if (strcmp(argv[i], "-seed") == 0 && argc > i) {
			seedval = atol(argv[++i]);
			fprintf(stderr, "Using seed %ld\n", seedval);
		}
	}

	if (width == 0 || height == 0 || dir == NULL) {
		fprintf(stderr, "generator -h <height> -w <width> -n <num puzzles> -d <directory> [-seed <seed>]\n");
		return 1;
	}

	if (seedval == -1)
		seedval = time(NULL);

	r = new Rand(seedval);

	for (unsigned int i=0; i<width*height; i++) {
		puzzle.push_back(i);
	}

	long unsigned int (*p_rand)(ptrdiff_t) = &getrand;

	for (unsigned int i = 0; i < n; i++) {
		unsigned int check = 1;

		while (check != 0) {
			random_shuffle(puzzle.begin(), puzzle.end(), p_rand);
			if (solvable(puzzle, width, height) == 0)
				check = 0;
			else
				unsolvable += 1;
		}
		writeruml(puzzle, width, height, dir, i);
	}

	fprintf(stderr, "%d puzzles were unsolvable\n", unsolvable);


	return 0;
}
