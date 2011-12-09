#include "image.hpp"
#include <iterator>
#include <vector>
#include <limits>
#include <cmath>
#include <cstdio>

void fatal(const char*, ...);

static inline bool inrange(double min, double max, double x) {
 	return x >= min - sqrt(std::numeric_limits<double>::epsilon())
		&& x <= max + sqrt(std::numeric_limits<double>::epsilon());
}

struct Point {

	static double dot(const Point &a, const Point &b) { return a.dot(b); }

	static Point sub(const Point &a, const Point &b) { return a.minus(b); }

	static double distance(const Point &a, const Point &b) {
		Point diff = b.minus(a);
		return sqrt(dot(diff, diff));
	}

	static Point inf(void) {
		return Point(std::numeric_limits<double>::infinity(),
			std::numeric_limits<double>::infinity());
	}

	static Point neginf(void) {
		return Point(-std::numeric_limits<double>::infinity(),
			-std::numeric_limits<double>::infinity());
	}

	// Get the angle off of the positive x axis
	// between 0 and 2π.
	static double angle(const Point &a) {
		double angle = atan2(a.y, a.x);
		if (angle < 0)
			return 2 * M_PI + angle;
		return angle;
	}

	// Get the clock-wise swing between uv and vw
	static double cwangle(const Point &u, const Point &v, const Point &w) {
		Point a = v.minus(u), b = w.minus(v);
		return -atan2(a.x*b.y - a.y*b.x, a.x*b.x+a.y*b.y);
	}

	Point(void) { }

	Point(double _x, double _y) : x(_x), y(_y) { }

	Point(double _x, double _y, double _z) : x(_x), y(_y) { }

	bool operator==(const Point &p) const {
		return fabs(x - p.x) < std::numeric_limits<double>::epsilon()
			&& fabs(y - p.y) < std::numeric_limits<double>::epsilon();
	}

	bool operator!=(const Point &p) const { return !(*this == p); }

	void draw(Image &img, Color c = Image::black, double r = 1) const {
		img.add(new Image::Circle(x, y, r,  c, -1));
	}

	double dot(const Point &b) const {
		return x * b.x + y * b.y;
	}

	Point minus(const Point &b) const {
		return Point(x - b.x, y - b.y);
	}

	double x, y;
};

struct Line {
	Line(void) { }

	Line(Point p0, Point p1) {
		double dx = p1.x - p0.x;
		double dy = p1.y - p0.y;
		if (fabs(dx) < std::numeric_limits<double>::epsilon()) {
			m = std::numeric_limits<double>::infinity();
			b = p1.x;
 		} else {
			m = dy / dx;
			b = p0.y - m * p0.x;
		}
	}

	// If they are parallel then (∞,∞), if they are
	// the same line then (FP_NAN, FP_NAN)
	Point isection(const Line &l) const {
		if (std::isinf(m) || std::isinf(l.m))
			return vertisect(*this, l);

		double x = (l.b - b) / (m - l.m);
		return Point(x, m * x + b);
	}

	bool isabove(const Point &p) const { return (m * p.x + b) < p.y; }

	// In case of a vertical line: m == ∞ and b = x
	double m, b;

private:
	static Point vertisect(const Line &a, const Line &b) {
		if (std::isinf(a.m) && std::isinf(b.m)) {
			if (fabs(a.b - b.b) < std::numeric_limits<double>::epsilon())
				return Point(FP_NAN, FP_NAN);
			return Point::inf();
		}

		const Line *v = &a, *l = &b;
		if (std::isinf(b.m)) {
			v = &b;
			l = &a;
		}

		return Point(v->b, l->m * v->b + l->b);
	}
};

struct LineSeg : public Line {
	LineSeg(void) { }

	LineSeg(Point _p0, Point _p1) : Line(_p0, _p1), p0(_p0), p1(_p1) {
		if (p1.x < p0.x) {
			mins.x = p1.x;
			maxes.x = p0.x;
		} else {
			mins.x = p0.x;
			maxes.x = p1.x;
		}
		if (p1.y < p0.y) {
			mins.y = p1.y;
			maxes.y = p0.y;
		} else {
			mins.y = p0.y;
			maxes.y = p1.y;
		}
	}

	void draw(Image &img, Color c = Image::black, double w = 1) const {
		img.add(new Image::Line(p0.x, p0.y, p1.x, p1.y, w, c));
	}

	double length(void) const {
		return Point::distance(p0, p1);
	}

	Point midpt(void) const {
		return Point((p0.x + p1.x) / 2, (p0.y + p1.y) / 2);
	}

	Point along(double dist) const {
		double theta = Point::angle(p1.minus(p0));
		return Point(p0.x + dist * cos(theta), p0.y + dist * sin(theta));
	}

	bool contains(const Point &p) const {
		if (fabs(m) < std::numeric_limits<double>::epsilon())
			return inrange(mins.x, maxes.x, p.x);
		return inrange(mins.x, maxes.x, p.x) && inrange(mins.y, maxes.y, p.y);
	}

	Point isection(const LineSeg &l) const {
		if (isvertical() || l.isvertical())
			return vertisect(*this, l);

		Point p = Line::isection(l);
		if (!contains(p) || !l.contains(p))
			return Point::inf();
		return p;
	}

	bool isvertical(void) const {
		return fabs(p0.x - p1.x) < std::numeric_limits<double>::epsilon();
	}

	Point p0, p1;
	Point mins, maxes;

private:

	// Deal with vertical line segments
	static Point vertisect(const LineSeg &a, const LineSeg &b) {
		if (a.isvertical() && b.isvertical())
			return Point::inf();

		const LineSeg *v = &a, *l = &b;
		if (b.isvertical()) {
			v = &b;
			l = &a;
		}
		Point p(v->p0.x, l->m * v->p0.x + l->b);
		if (!a.contains(p) || !b.contains(p))
			return Point::inf();

		return p;
	}
};

struct Bbox {
	Bbox(void) : min(Point::inf()), max(Point::neginf()) { }

	Bbox(const Point &_min, const Point &_max) : min(_min), max(_max) { }

	// Get the bounding box for the set of points
	Bbox(std::vector<Point> &pts) {
		min = Point::inf();
		max = Point::neginf();

		for (unsigned int i = 0; i < pts.size(); i++) {
			Point &p = pts[i];
			if (p.x > max.x)
				max.x = p.x;
			if (p.x < min.x)
				min.x = p.x;
			if (p.y > max.y)
				max.y = p.y;
			if (p.y < min.y)
				min.y = p.y;
		}
	}

	void draw(Image&, Color c = Image::black, double lwidth = 1) const;

	bool contains(const Point &p) const {
		return inrange(min.x, max.x, p.x) && inrange(min.y, max.y, p.y);
	}

	bool hits(const Bbox &b) const {
		return !(min.x > b.max.x || b.min.x > max.x
			|| min.y > b.max.y || b.min.y > max.y);
	}

	void move(double dx, double dy) {
		min.x += dx;
		max.x += dx;
		min.y += dy;
		max.y += dy;
	}

	Point min, max;
};

struct Polygon {

	// Generates a random polygon for which no
	// side crosses the line between the vertices
	// the minimum and maximum x value.
	static Polygon random(unsigned int n, double xc, double yc, double r);

	// Return the polygon that is the outter-hull
	// of the given set of points.
	static Polygon giftwrap(const std::vector<Point>&);

	Polygon(const std::vector<Point>&);

	Polygon(unsigned int, ...);

	Polygon(FILE*);

	void output(FILE*) const;

	// If the lwidth is <0 then the polygon is filled.
	void draw(Image&, Color c = Image::black, double lwidth = 1) const;

	bool contains(const Point&) const;

	bool hits(const LineSeg &) const;

	Point minisect(const LineSeg&) const;

	std::vector<Point> isections(const LineSeg&) const;

	// Indices of reflex vertices.
	std::vector<unsigned int> reflexes(void) const;

	void scale(double f) {
		for (unsigned long i = 0; i < verts.size(); i++) {
			verts[i].x *= f;
			verts[i].y *= f;
		}
		bbox = Bbox(verts);
		initsides();
	}

	void move(double dx, double dy) {
		bbox.move(dx, dy);
		for (unsigned int i = 0; i < verts.size(); i++) {
			verts[i].x += dx;
			verts[i].y += dy;
		}
		initsides();
	}

	std::vector<Point> verts;
	std::vector<LineSeg> sides;
	Bbox bbox;

private:
	double interangle(unsigned int) const;
	bool isreflex(unsigned int) const;
	void collapseflats(void);
	void initsides(void);
};