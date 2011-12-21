#ifndef _BODY_HPP_
#define _BODY_HPP_

#include <cassert>
#include "../utils/geom.hpp"

struct Lvl;

struct Line1d {
	Line1d(double _a, double _b) : a(_a), b(_b) { }

	double isection(const Line1d &o) const {
		if (between(o.a, o.b, b))
			return b - o.a;
		else if (between(a, b, o.b))
			return o.b - a;
		return -1.0;
	}

	double a, b;
};

struct Isect {
	Isect(void) : is(false), dx(0.0), dy(0.0) { }

	Isect(double _dx, double _dy) : is(true), dx(_dx), dy(_dy) { }

	Isect(const Isect &o) : is(o.is), dx(o.dx), dy(o.dy) { }

	double area(void) { return dx * dy; }

	bool is;
	double dx, dy;
};

struct Bbox : public Rectangle {

	Bbox(void) { }

	Bbox(unsigned int x0, unsigned int y0, unsigned int x1,
			unsigned int y1) : Rectangle(x0, y0, x1, y1) { }

	Isect isection(const Bbox &o) const {
		double ix = projx().isection(o.projx());
		if(ix > 0.0) {
			double iy = projy().isection(o.projy());
			if(iy > 0.0)
				return Isect(ix, iy);
		}
		return Isect();
	}

	Line1d projx(void) const { return Line1d(min.x, max.x); }

	Line1d projy(void) const { return Line1d(min.y, max.y); }
};

struct Body {
	enum { Maxdy = 12 };

	Body(void) { }

	Body(unsigned int x, unsigned int y, unsigned int _z,
			unsigned int w, unsigned int h) :
		bbox(x, y, x + w, y + h), dy(0), z(_z), fall(false) { }

	Body(const Body &o) :
		bbox(o.bbox), dy(o.dy), z(o.z), fall(o.fall) { }

	bool operator==(const Body &o) const {
		return z == o.z && fall == o.fall && bbox == o.bbox &&
			doubleeq(dy, o.dy);
	}

	void move(const Lvl&, double dx);
	
	Bbox bbox;
	double dy;
	unsigned char z;
	bool fall;

private:

	Point step(const Point&);
	void dofall(const Lvl&, const Isect&);
};

#endif	// _BODY_HPP_