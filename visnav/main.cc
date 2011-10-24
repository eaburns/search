#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"
#include <limits>

enum {
	Width = 800,
	Height = 800,
	Npolys = 500,
	Maxverts = 8,
};

static const double Minrad = 50;
static const double Maxrad = 75;


struct Dims {
	Dims(void) :
		minx(std::numeric_limits<double>::infinity()),
		maxx(-std::numeric_limits<double>::infinity()),
		miny(std::numeric_limits<double>::infinity()),
		maxy(-std::numeric_limits<double>::infinity())
	{}

	void update(double x, double y) {
		if (x < minx)
			minx = x;
		if (x > maxx)
			maxx = x;
		if (y < miny)
			miny = y;
		if (y > maxy)
			maxy = y;
	}

	double minx, maxx, miny, maxy;
};

Dims randpolys(std::vector<Polygon>&, unsigned int n, unsigned int maxverts,
		double width, double height, double minrad, double maxrad);

int main(int argc, char *argv[]) {
	printf("seed=%lu\n", randgen.seed());

	double start = walltime();
	std::vector<Polygon> polys;
	Dims dims = randpolys(polys, Npolys, Maxverts, Width, Height, Minrad, Maxrad);
	printf("Generated %u polygons: %g seconds\n", Npolys, walltime() - start);

	start = walltime();
	VisGraph graph(polys);
	printf("Computed visibility graph: %g seconds\n", walltime() - start);

	Image img(dims.maxx - dims.minx, dims.maxy - dims.miny, "poly.eps");
	graph.draw(img);
	img.save("polys.eps", true);

	return 0;
}

Dims randpolys(std::vector<Polygon> &polys, unsigned int n, unsigned int maxverts,
		double width, double height, double minrad, double maxrad) {
	Dims dims;

	for (unsigned int i = 0; i < Npolys; i++) {
redo:
		Polygon p = Polygon::random(
				randgen.integer(3, maxverts),
				randgen.real() * width,
				randgen.real() * height,
				randgen.real() * (maxrad - minrad) + minrad);

		for (unsigned int i = 0; i < polys.size(); i++) {
			if (polys[i].bbox.isect(p.bbox))
				goto redo;
		}

		dims.update(p.bbox.min.x, p.bbox.min.y);
		dims.update(p.bbox.max.x, p.bbox.max.y);
		polys.push_back(p);
	}

	for (unsigned int i = 0; i < polys.size(); i++)
		polys[i].move(-dims.minx, -dims.miny);

	return dims;
}