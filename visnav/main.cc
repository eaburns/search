#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"
#include <limits>

enum {
	Width = 1000,
	Height = 1000,
	Npolys = 2,
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

Dims randpolys(std::vector<Poly>&, unsigned int n, unsigned int maxverts,
		double width, double height, double minrad, double maxrad);

void addline(Image&, Line&, Color c = Image::black);

int main(int argc, char *argv[]) {
	printf("seed=%lu\n", randgen.seed());

	double start = walltime();
	std::vector<Poly> polys;
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

Dims randpolys(std::vector<Poly> &polys, unsigned int n, unsigned int maxverts,
		double width, double height, double minrad, double maxrad) {
	Dims dims;

	for (unsigned int i = 0; i < Npolys; i++) {
redo:
		Poly p = Poly::random(
				randgen.integer(3, maxverts),
				randgen.real() * width,
				randgen.real() * height,
				randgen.real() * (maxrad - minrad) + minrad);

		for (unsigned int j = 0; j < p.verts.size(); j++) {
			dims.update(p.verts[j].x, p.verts[j].y);
			unsigned int nxt = j == p.verts.size() - 1 ? 0 : j + 1;

			for (unsigned int k = 0; k < polys.size(); k++) {
				if (Point::distance(polys[k].centroid(), p.centroid()) > maxrad)
					continue;

				if (polys[k].contains(p.verts[j]))
					goto redo;

				Line side(p.verts[j], p.verts[nxt]);
				if (!isinf(polys[k].minhit(side)))
					goto redo;
			}
		}
		polys.push_back(p);
	}

	for (unsigned int i = 0; i < polys.size(); i++) {
	for (unsigned int j = 0; j < polys[i].verts.size(); j++) {
		polys[i].verts[j].x -= dims.minx;
		polys[i].verts[j].y -= dims.miny;
	}
	}

	return dims;
}

void addline(Image &img, Line &line, Color c) {
	Image::Path *p = new Image::Path();
	p->setlinewidth(0.1);
	p->setcolor(c);
	p->line(line.p0.x, line.p0.y, line.p1.x, line.p1.y);
	img.add(p);
}
