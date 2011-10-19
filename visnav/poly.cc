#include "poly.hpp"
#include "../utils/image.hpp"
#include "../utils/utils.hpp"
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdarg>
#include <cassert>
#include <cstdio>

static void xsortedpts(std::vector<Point>&, double, double, double);
static void swp(double*, double*);
static double angle(const Point&, const Point&);

void Line::init(double x0, double y0, double x1, double y1) {
	if (x0 > x1) {
		swp(&x0, &x1);
		swp(&y0, &y1);
	}
	m = (y1 - y0) /  (x1 - x0); 
	b = y0 - (m * x0);
	theta = angle(p0, p1);

	minx = p0.x;
	maxx = p1.x;
	if (minx > maxx)
		swp(&minx, &maxx);

	miny = p0.y;
	maxy = p1.y;
	if (miny > maxy)
		swp(&miny, &maxy);
}

Point Line::intersection(const Line &a, const Line &b) {
	double x = (b.b - a.b) / (a.m - b.m);
	double y = a.m * x + a.b;
	return Point(x, y);
}

Poly::Poly(unsigned int nverts, ...) {
	va_list ap;
	va_start(ap, nverts);
	for (unsigned int i = 0; i < nverts; i++) {
		Point p;
		p.x = va_arg(ap, double);
		p.y = va_arg(ap, double);
		verts.push_back(p);
	}
	va_end(ap);
}

Poly::Poly(unsigned int nverts, va_list ap) {
	for (unsigned int i = 0; i < nverts; i++) {
		Point p;
		p.x = va_arg(ap, double);
		p.y = va_arg(ap, double);
		verts.push_back(p);
	}
}

Poly Poly::random(unsigned int n, double xc, double yc, double r) {
	if (n < 3)
		fatal("Unable to create a polygon with fewer than 3 vertices\n");

	std::vector<Point> pts(n);;
	xsortedpts(pts, xc, yc, r);

	std::vector<Point> verts;
	verts.push_back(pts[0]);

	Line l(pts[0], pts[pts.size()-1]);
	for (unsigned int i = 1; i < pts.size() - 1; i++) {
		if (l.above(pts[i]))
			verts.push_back(pts[i]);
	}

	verts.push_back(pts[pts.size() -1]);
	for (unsigned int i = pts.size() - 2; i >= 1; i--) {
		if (!l.above(pts[i]))
			verts.push_back(pts[i]);
	}

	return Poly(verts);
}

Poly Poly::triangle(double x, double y, double radius) {
	return Poly(3,
		x, y + radius,
		x + radius * cos(7 * M_PI / 6), y + radius * sin(7 * M_PI / 6),
		x + radius * cos(11 * M_PI / 6), y + radius * sin(11 * M_PI / 6));
}

Poly Poly::square(double x, double y, double height) {
	return Poly(4,
			x + height / 2, y + height / 2,
			x - height / 2, y + height / 2,
			x - height / 2, y - height / 2,
			x + height / 2, y - height / 2);
}

enum { Bufsz = 10 };

void Poly::draw(Image &img, Color c, double width, bool number) const {
	Image::Path *p = new Image::Path();
	p->setcolor(c);
	if (width >= 0)
		p->setlinewidth(width);
	else
		p->setlinewidth(0.1);
	p->moveto(verts[0].x, verts[0].y);
	for (unsigned int i = 1; i < verts.size(); i++)
		p->lineto(verts[i].x, verts[i].y);
	p->closepath();
	if (width < 0)
		p->fill();
	img.add(p);

	if (!number)
		return;

	char buf[Bufsz];
	for (unsigned int i = 0; i < verts.size(); i++) {
		snprintf(buf, Bufsz, "%u", i);
		img.add(new Image::Text(buf, verts[i].x, verts[i].y, 6));
	}
}

bool Poly::willhit(const Line &l) const {
	double min = std::numeric_limits<double>::infinity();
	double max = -std::numeric_limits<double>::infinity();

	for (unsigned int i = 0; i < verts.size(); i++) {
		double theta = angle(l.p0, verts[i]);
		if (theta < min)
			min = theta;
		if (theta > max)
			max = theta;
	}

	return l.theta >= min && l.theta <= max;
}

double Poly::minhit(const Line &line) const {
	if (!willhit(line))
		return std::numeric_limits<double>::infinity();
 
	double min = std::numeric_limits<double>::infinity();
	for (unsigned int i = 0; i < verts.size(); i++) {
		Line side(i == 0 ? verts[verts.size() - 1] : verts[i-1], verts[i]);
		Point hitpt = Line::intersection(line, side);

		if (!side.contains(hitpt))
			continue;

		double hitdist = Point::distance(line.p0, hitpt);
		if (hitdist < min)
			min = hitdist;
	}

	return min;
}

struct CmpX {
	bool operator()(const Point &a, const Point &b) {
		return a.x < b.x;
	}
} increasingX;

static void xsortedpts(std::vector<Point> &pts, double xc, double yc, double r) {
	for (unsigned int i = 0; i < pts.size(); i++) {
		double x = (2 * r * randgen.real()) - r;
		double y = (2 * r * randgen.real()) - r;
		pts[i] = Point(x + xc, y + yc);
	}

	std::sort(pts.begin(), pts.end(), increasingX);
}

static void swp(double *a, double *b) {
	double t = *a;
	*a = *b;
	*b = t;
}

static double angle(const Point &a, const Point &b) {
	double dx = b.x - a.x;
	double dy = b.y - a.y;
	double hyp = sqrt(dx * dx + dy * dy);
	return asin(dy / hyp);
}