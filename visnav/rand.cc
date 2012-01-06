#include "polymap.hpp"
#include "../utils/utils.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>

static void randpolys(std::vector<Polygon>&);
static double rnddbl(double, double);
static void helpmsg(int);

static unsigned int Npolys = 500;
static unsigned int Maxverts = 8;
static double Minrad = 0.05;
static double Maxrad = 0.075;
static const char *outfile;
static bool stats;

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0)
			helpmsg(0);
		else if (i < argc - 1 && strcmp(argv[i], "-o") == 0)
			outfile = argv[++i];
		else if (i < argc - 1 && strcmp(argv[i], "-npolys") == 0)
			Npolys = strtoll(argv[++i], NULL, 10);
		else if (i < argc - 1 && strcmp(argv[i], "-nverts") == 0)
			Maxverts = strtoll(argv[++i], NULL, 10);
		else if (i < argc - 1 && strcmp(argv[i], "-minrad") == 0)
			Minrad = strtod(argv[++i], NULL);
		else if (i < argc - 1 && strcmp(argv[i], "-maxrad") == 0)
			Maxrad = strtod(argv[++i], NULL);
		else if (strcmp(argv[i], "-stats") == 0)
			stats = true;
		else
			helpmsg(1);
	}

	if (stats)
		fprintf(stderr, "seed: %lu\n", randgen.seed());

	std::vector<Polygon> polys;
	double start = walltime();
	randpolys(polys);
	if (stats)
		fprintf(stderr, "created %u polys in %gs\n", Npolys, walltime() - start);

	PolyMap map(polys);
	if (outfile) {
		FILE *f = fopen(outfile, "w");
		if (!f)
			fatalx(errno, "Failed to open %s:", outfile);
		map.output(stdout);
		fclose(f);
	} else {
		map.output(stdout);
	}

	return 0;
}

static void randpolys(std::vector<Polygon> &polys) {
	for (unsigned int i = 0; i < Npolys; i++) {
redo:
		double r = rnddbl(Minrad, Maxrad);
		double x = rnddbl(r, 1 - r);
		double y = rnddbl(r, 1 - r);

		assert (x >= 0.0);
		assert (y >= 0.0);
		assert (r >= 0.0);

		Polygon p = Polygon::random(randgen.integer(3, Maxverts), x, y, r);
			
		for (unsigned int i = 0; i < polys.size(); i++) {
			if (polys[i].bbox.hits(p.bbox))
				goto redo;
		}

		polys.push_back(p);
	}
}

static double rnddbl(double min, double max) {
	assert (max > min);
	return randgen.real() * (max - min) + min;
}

static void helpmsg(int res) {
	puts("Usage: rand [options]");
	puts("\nPlaces random polygons in the unit square.  Computes the");
	puts("visibility graph and prints the instance to standard output");
	puts("\nOptions:");
	puts("	-h	prints this help message");
	puts("	-o <file>	output to the specified file instead of standard output");
	puts("	-npolys <num>	specify the number of polygons (default: 500)");
	puts("	-nverts <num>	specify the max number of vertices (default: 8)");
	puts("	-minrad <num>	minimum polygon radius (default: 0.05)");
	puts("	-maxrad <num>	maximum polygon radius (default: 0.075)");
	puts("	-stats	print some stastics to standard error");
	exit(res);
}