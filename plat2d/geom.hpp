#ifndef _GEOM_HPP_
#define _GEOM_HPP_

#include <cassert>

struct Lvl;

static bool between(double min, double max, double n) {
	return n >= min && n <= max;
}

struct Point {
	Point(double a, double b) : x(a), y(b) { }

	Point(const Point &o) : x(o.x), y(o.y) { }

	void move(double dx, double dy) {
		x += dx;
		y += dy;
	}

	double x, y;
};

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

struct Rect {

	Rect(double x0, double y0, double x1, double y1) :
		a(x0, y0), b(x1, y1) { normalize(); }

	Rect(const Rect &o) : a(o.a), b(o.b) { }

	void move(double dx, double dy) {
		a.move(dx, dy);
		b.move(dx, dy);
	}

	bool contains(Point p) const {
		return between(a.x, b.x, p.x) &&
			between(b.x, b.y, p.y);
	}

	Isect isection(const Rect &o) const {
		double ix = projx().isection(o.projx());
		if(ix > 0.0) {
			double iy = projy().isection(o.projy());
			if(iy > 0.0)
				return Isect(ix, iy);
		}
		return Isect();
	}

	Line1d projx(void) const {
		assert (a.x <= b.x);
		return Line1d(a.x, b.x);
	}

	Line1d projy(void) const {
		assert (a.y <= b.y);
		return Line1d(a.y, b.y);
	}

	Point a, b;

private:

	void normalize(void) {
		if (a.x > b.x) {
			double t = a.x;
			a.x = b.x;
			b.x = t;
		}
		if (a.y > b.y) {
			double t = a.y;
			a.y = b.y;
			b.y = t;
		}
	}
};

struct Body {
	enum { Maxdy = 12 };

	Body(void) : z(0), bbox(0, 0, 1, 1), vel(0, 0), acc(0, 0), fall(false) { }

	Body(unsigned int x, unsigned int y, unsigned int _z,
			unsigned int w, unsigned int h) :
		z(_z), bbox(x, y, x + w, y + h), vel(0, 0), acc(0, 0), fall(false) { }

	Body(const Body &o) : z(o.z), bbox(o.bbox), vel(o.vel), acc(o.acc), fall(o.fall) { }

	void move(const Lvl&);
	
	unsigned int z;
	Rect bbox;
	Point vel, acc;
	bool fall;

private:

	Point step(const Point&);
	void dofall(const Lvl&, const Isect&);
};

#endif	// _GEOM_HPP_