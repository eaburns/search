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

static void parseargs(int, char*[]);
static void helpmsg(int);

enum {
	// BlkSz is the size (in points or something) of each
	// grid block in the output image.
	BlkSz = 1,
};

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
	Image img(map.w*BlkSz, map.h*BlkSz, mappath);

	for (unsigned int x = 0; x < map.w; x++) {
	for (unsigned int y = 0; y < map.h; y++) {
		if (!map.blkd(map.index(x, y)))
			continue;

		for (unsigned int i = 0; i < BlkSz; i++) {
		for (unsigned int j = 0; j < BlkSz; j++) {
			img.setpixel(x*BlkSz + i, map.h - (y*BlkSz + j) -1, Image::black);
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

	return 0;
}

static void parseargs(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);

		}else if (i < argc-1 && strcmp(argv[i], "-o") == 0) {
			outpath = argv[++i];

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
	puts("-o <path>	specify the output path, '-' for standard output");
	exit(status);
}