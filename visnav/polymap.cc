#include "polymap.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"
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
		bound = Bound(Polygon(in));

	unsigned int npoly;
	res = fscanf(in, " %u polygons\n", &npoly);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the the visibility map");
	if (res != 1)
		fatal("Malformed visibility map");

	for (unsigned int i = 0; i < npoly; i++)
		polys.push_back(Polygon(in));
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

void PolyMap::draw(Image &img, double lwidth) const {
	for (unsigned int i = 0; i < polys.size(); i++) {
		const Color &c = somecolors[i % Nsomecolors];
		polys[i].draw(img, c, lwidth);
	}
	if (bound)
		bound->draw(img, Color(0, 0, 0), lwidth);
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

Point PolyMap::min(void) const {
	if (bound)
		return bound->bbox.min;

	Point min = Point::inf();
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (polys[i].bbox.min.x < min.x)
			min.x = polys[i].bbox.min.x;
		if (polys[i].bbox.min.y < min.y)
			min.y = polys[i].bbox.min.y;
	}
	return min;
}

Point PolyMap::max(void) const {
	if (bound)
		return bound->bbox.max;

	Point max = Point::neginf();
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (polys[i].bbox.max.x > max.x)
			max.x = polys[i].bbox.max.x;
		if (polys[i].bbox.min.y > max.y)
			max.y = polys[i].bbox.max.y;
	}
	return max;
}

bool PolyMap::isvisible(const Point &a, const Point &b) const {
	const LineSeg line(a, b);
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (!line.bbox.hits(polys[i].bbox))
			continue;
		if (polys[i].hits(line))
			return false;
	}
	if (!bound)
		return true;

 	const Polygon &bnd = *bound;
	return bnd.contains(a) && !bnd.hits(line);
}
