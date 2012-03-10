#include "visgraph.hpp"
#include "../utils/image.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cstring>
#include <cerrno>

static void helpmsg(int);

static const char *infile;
static const char *outfile;
static bool echo, labels;

enum {
	Width = 400,
	Height = 400,
};

int main(int argc, char *argv[]) {
	for (unsigned int i = 1; i < (unsigned int) argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);
		} else if (strcmp(argv[i], "-e") == 0) {
			echo = true;
		} else if (i < (unsigned int) argc - 1 && strcmp(argv[i], "-o") == 0) {
			outfile = argv[++i];
		} else if (i < (unsigned int) argc - 1 && strcmp(argv[i], "-i") == 0) {
			infile = argv[++i];
		} else if (strcmp(argv[i], "-l") == 0) {
			labels = true;
		} else {
			printf("Unknown option: %s\n", argv[i]);
			helpmsg(1);
		}
	}

	if (!outfile) {
		printf("No output file specified\n");
		helpmsg(1);
	}

	FILE *in = stdin;
	if (infile) {
		in = fopen(infile, "r");
		if (!in)
			fatalx(errno, "Failed to open %s", infile);
	}

	VisGraph graph(in);
	if (echo)
		graph.output(stdout);

	if (labels)
		graph.dumpvertlocs(stderr);

	Geom2d::Pt min = graph.map.min();
	Geom2d::Pt max = graph.map.max();
	graph.translate(-min.x, -min.y);
	double w = max.x - min.x;
	double h = max.y - min.y;
	double sx = Width / w, sy = Height / h;
	if (sx < sy)
		graph.scale(sx, sx);
	else
		graph.scale(sy, sy);

	Image img(Width, Height);
	graph.draw(img, labels);
	img.saveeps(outfile, true, 72.0/2.0);

	if (infile)
		fclose(in);

	return 0;
}

static void helpmsg(int ret) {
	puts("Usage: draw -o <outfile> [options]");
	puts("Options:");
	puts("	-h	print this help message");
	puts("	-i <infile>	read input from file");
	puts("	-e	echo input to standard output");
	puts("	-l	draw vertex labels and dump their locations to stderr");
	exit(ret);
}
