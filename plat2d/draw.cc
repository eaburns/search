#include "lvl.hpp"
#include "../utils/image.hpp"
#include <cstring>

const char *outfile;

static void parseargs(int, char*[]);
static void helpmsg(int);

int main(int argc, char *argv[]) {
	parseargs(argc, argv);

	Lvl lvl(stdin);

	Image img(
		lvl.width() * Tile::Width,
		lvl.height() * Tile::Height,
		outfile);

	lvl.draw(img);
	img.save(outfile, true);

	return 0;
}

static void parseargs(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0 && i < argc - 1) {
			outfile = argv[++i];
		} else if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);
		} else {
			printf("Unknown option: %s\n", argv[i]);
			helpmsg(1);
		}
	}

	if (!outfile) {
		printf("No output file specified\n");
		helpmsg(1);
	}
}

static void helpmsg(int status) {
	puts("Usage: draw -o <outfile> [args]");
	puts("Options:");
	puts("-h	display this help message");
	puts("-o <outfile>	set the output file");
	exit(status);
}