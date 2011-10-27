#include "visgraph.hpp"
#include "../utils/utils.hpp"

enum {
	Npolys = 500,
	Maxverts = 8,
};

static const double Minrad = 0.05;
static const double Maxrad = 0.075;

void randpolys(std::vector<Polygon>&);
static double rnddbl(double, double);

int main(int argc, char *argv[]) {
	std::vector<Polygon> polys;
	randpolys(polys);
	VisGraph graph(polys);
	graph.output(stdout);
	return 0;
}

void randpolys(std::vector<Polygon> &polys) {
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
			if (polys[i].bbox.isect(p.bbox))
				goto redo;
		}

		polys.push_back(p);
	}
}

static double rnddbl(double min, double max) {
	assert (max > min);
	return randgen.real() * (max - min) + min;
}