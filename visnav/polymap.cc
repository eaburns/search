#include "polymap.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"
#include <cerrno>

PolyMap::PolyMap(FILE *in) {
	input(in);
}

void PolyMap::input(FILE *in) {
	unsigned int npoly;
	int res = fscanf(in, " %u polygons\n", &npoly);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the the visibility map");
	if (res != 1)
		fatal("Malformed visibility map");

	for (unsigned int i = 0; i < npoly; i++)
		polys.push_back(Polygon(in));
}

void PolyMap::output(FILE *out) const {
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
}

void PolyMap::scale(double sx, double sy) {
	for (unsigned int i = 0; i < polys.size(); i++)
		polys[i].scale(sx, sy);		
}

void PolyMap::translate(double dx, double dy) {
	for (unsigned int i = 0; i < polys.size(); i++)
		polys[i].translate(dx, dy);		
}

Point PolyMap::min(void) const {
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
	Rectangle bbox(a, b);
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (!bbox.hits(polys[i].bbox))
			continue;
		if (polys[i].hits(line))
			return false;
	}
	return true;
}
