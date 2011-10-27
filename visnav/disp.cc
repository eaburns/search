#include "visgraph.hpp"
#include "../utils/image.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cstring>
#include <cerrno>

static void usage(int);

static const char *infile;
static const char *outfile;
static bool echo;

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0)
			usage(0);
		else if (strcmp(argv[i], "-e") == 0)
			echo = true;
		else if (i < argc - 1 && strcmp(argv[i], "-o") == 0)
			outfile = argv[++i];
		else if (i < argc - 1 && strcmp(argv[i], "-i") == 0)
			infile = argv[++i];
	}

	if (!outfile)
		usage(1);
	if (echo)
		fatal("-e is not yet implemented");

	FILE *in = stdin;
	if (infile) {
		in = fopen(infile, "r");
		if (!in)
			fatalx(errno, "Failed to open %s", infile);
	}

	VisGraph graph(in);
	Image img(400, 400);
	graph.draw(img, 400);
	img.save(outfile, true, 72.0/2.0);

	if (infile)
		fclose(in);

	return 0;
}

static void usage(int ret) {
	printf("Usage: disp -o <outfile> [options]\n");
	printf("Options:\n");
	printf("	-h	print this help message\n");
	printf("	-i <infile>	read input from file\n");
	printf("	-e	echo input to standard output\n");
	exit(ret);
}