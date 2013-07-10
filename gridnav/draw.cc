// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include "../utils/geom2d.hpp"
#include "../graphics/image.hpp"
#include <string>
#include <cstring>
#include <cstdio>
#include <cerrno>

static const char *mappath;
static const char *outpath;
static bool print;
static unsigned int cellsz = 1;


static void parseargs(int, char*[]);
static void helpmsg(int);

int main(int argc, char *argv[]) {
	parseargs(argc, argv);

	FILE *mapfile = stdin;
	if (mappath != NULL) {
		mapfile = fopen(mappath, "r");
		if (!mapfile)
			fatalx(errno, "Failed to open %s for reading", mapfile);
	} else {
		mappath = "<untitled>";
	}
	GridMap map(mapfile);

	Image img(map.w*cellsz, map.h*cellsz, mappath);

	for (unsigned int x = 0; x < map.w; x++) {
	for (unsigned int y = 0; y < map.h; y++) {
		if (!map.blkd(map.index(x, y)))
			continue;

		Color c = Image::black;
		if (x == 1 && y == 1)
			c = Image::green;
		else if (x == map.w-2 && y == 1)
			c = Image::red;

		for (unsigned int i = 0; i < cellsz; i++) {
		for (unsigned int j = 0; j < cellsz; j++) {
			img.setpixel(x*cellsz + i, (map.h - y - 1)*cellsz + j, c);
		}
		}
	}
	}

	FILE *outfile = stdout;
	if (outpath != NULL && strcmp(outpath, "-") != 0) {
		outfile = fopen(outpath, "w");
		if (!outfile)
			fatalx(errno, "Failed to open %s for writing", outpath);
	}
	img.writeeps(outfile);

	if (print) {
		for (unsigned int y = 0; y < map.h; y++) {
			for (unsigned int x = 0; x < map.w; x++) {
				if (!map.blkd(map.index(x, y)))
					putc(' ', stdout);
				else
					putc('#', stdout);
			}
			putc('\n', stdout);
		}
	}

	return 0;
}

static void parseargs(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);

		}else if (strcmp(argv[i], "-p") == 0) {
			print = true;

		}else if (i < argc-1 && strcmp(argv[i], "-o") == 0) {
			outpath = argv[++i];

		}else if (i < argc-1 && strcmp(argv[i], "-cellsz") == 0) {
			cellsz = strtol(argv[++i], NULL, 0);

		} else if (mappath != NULL) {
			helpmsg(1);

		} else {
			mappath = argv[i];
		}
	}
}

static void helpmsg(int status) {
	puts("Usage: draw [options] <map path>");
	puts("-h	print this help message");
	puts("-p	print the grid to standard output too");
	puts("-o <path>	specify the output path, '-' for standard output");
	puts("-cellsz <number>	specify the size (in pixels) of each grid cell");
	exit(status);
}