#pragma once

#include <cassert>
#include "../utils/geom2d.hpp"

struct Lvl;

struct Line1d {
	Line1d(double i, double j) : a(i), b(j) { }

	double isect(const Line1d &o) const {
		if (geom2d::between(o.a, o.b, b))
			return b - o.a;
		else if (geom2d::between(a, b, o.b))
			return o.b - a;
		return -1.0;
	}

	double a, b;
};

struct Isect {
	Isect() : is(false), dx(0.0), dy(0.0) { }

	Isect(double delatx, double deltay) : is(true), dx(delatx), dy(deltay) { }

	Isect(const Isect &o) : is(o.is), dx(o.dx), dy(o.dy) { }

	double area() { return dx * dy; }

	bool is;
	double dx, dy;
};

struct Bbox : public geom2d::Bbox {

	Bbox() { }

	Bbox(unsigned int x0, unsigned int y0, unsigned int x1,
			unsigned int y1) : geom2d::Bbox(x0, y0, x1, y1) { }

	Isect isect(const Bbox &o) const {
		double ix = projx().isect(o.projx());
		if(ix > 0.0) {
			double iy = projy().isect(o.projy());
			if(iy > 0.0)
				return Isect(ix, iy);
		}
		return Isect();
	}

	Line1d projx() const { return Line1d(min.x, max.x); }

	Line1d projy() const { return Line1d(min.y, max.y); }
};

struct Body {
	enum { Maxdy = 12 };

	Body() { }

	Body(unsigned int x, unsigned int y, unsigned int w, unsigned int h) :
		bbox(x, y, x + w, y + h), dy(0), fall(false) { }

	Body(const Body &o) : bbox(o.bbox), dy(o.dy), fall(o.fall) { }

	bool operator==(const Body &o) const {
		return fall == o.fall && bbox == o.bbox && geom2d::doubleeq(dy, o.dy);
	}

	void output(FILE *out) const {
		fprintf(out, "%g,%g,	dy=%g,	fall=%d\n", bbox.min.x, bbox.min.y,
			dy, fall);
	}

	void move(const Lvl&, double dx);
	
	Bbox bbox;
	double dy;
	bool fall;

private:

	geom2d::Pt step(const geom2d::Pt&);

	void dofall(const Lvl&, const Isect&);
};
