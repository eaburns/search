#include "polymap.hpp"
#include "../utils/utils.hpp"
#include "../graphics/image.hpp"
#include <cerrno>
#include <cstring>

PolyMap::PolyMap(FILE *in) {
	input(in);
}

void PolyMap::input(FILE *in) {
	int n = 0;
	int res = fscanf(in, "bound: %n", &n);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the visibility map");
	if (res != 0)
		fatal("Malformed visibility map");
	if (n == strlen("bound: "))
		bound = Bound(geom2d::Poly(in));

	unsigned int npoly;
	res = fscanf(in, " %u polygons\n", &npoly);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the the visibility map");
	if (res != 1)
		fatal("Malformed visibility map");

	for (unsigned int i = 0; i < npoly; i++)
		polys.push_back(geom2d::Poly(in));
}

void PolyMap::output(FILE *out) const {
	if (bound) {
		fprintf(out, "bound: ");
		bound->output(out);
		putc('\n', out);
	}
	fprintf(out, "%u polygons\n", (unsigned int) polys.size());
	for (unsigned int i = 0; i < polys.size(); i++) {
		polys[i].output(out);
		putc('\n', out);
	}
}

void PolyMap::draw(Image &img, double w) const {
	for (unsigned int i = 0; i < polys.size(); i++) {
		const Color &c = somecolors[i % Nsomecolors];
		img.add(new Image::Poly(polys[i], c, w));
	}
	if (bound) {
		if (w < 1)	// Filling in the bounding polygon fills the wrong side.
			w = 1;
		img.add(new Image::Poly(*bound, Image::black, w));
	}
}

void PolyMap::scale(double sx, double sy) {
	for (unsigned int i = 0; i < polys.size(); i++)
		polys[i].scale(sx, sy);
	if (bound)
		bound->scale(sx, sy);	
}

void PolyMap::translate(double dx, double dy) {
	for (unsigned int i = 0; i < polys.size(); i++)
		polys[i].translate(dx, dy);
	if (bound)
		bound->translate(dx, dy);
}

geom2d::Pt PolyMap::min() const {
	if (bound)
		return bound->bbox.min;

	geom2d::Pt min = geom2d::Pt::inf();
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (polys[i].bbox.min.x < min.x)
			min.x = polys[i].bbox.min.x;
		if (polys[i].bbox.min.y < min.y)
			min.y = polys[i].bbox.min.y;
	}
	return min;
}

geom2d::Pt PolyMap::max() const {
	if (bound)
		return bound->bbox.max;

	geom2d::Pt max = geom2d::Pt::neginf();
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (polys[i].bbox.max.x > max.x)
			max.x = polys[i].bbox.max.x;
		if (polys[i].bbox.min.y > max.y)
			max.y = polys[i].bbox.max.y;
	}
	return max;
}

bool PolyMap::isvisible(const geom2d::Pt &a, const geom2d::Pt &b) const {
	const geom2d::LineSg line(a, b);
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (!line.bbox.hits(polys[i].bbox))
			continue;
		if (polys[i].hits(line))
			return false;
	}
	if (!bound)
		return true;

 	const geom2d::Poly &bnd = *bound;
	return bnd.contains(a) && !bnd.hits(line);
}
