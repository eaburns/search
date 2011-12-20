#include "image.hpp"
#include <iterator>
#include <vector>
#include <limits>
#include <cmath>
#include <cstdio>

extern void fatal(const char*, ...);	// from utils.hpp

// Epsilon is the smallest double value that can be added
// to 1 to make it no longer equal to 1.
static const double Epsilon = std::numeric_limits<double>::epsilon();

// Inifinity is the double representation of inifinity.
static const double Infinity = std::numeric_limits<double>::infinity();

// doubleeq is a double equality test that does not use ==, instead
// it performs an approximate equality test for within
// 10·Epsilon.
static inline bool doubleeq(double a, double b) {
	return fabs(a - b) < 10 * Epsilon ||
		(std::isnan(a) && std::isnan(b)) ||
		(std::isinf(a) && std::isinf(b));
}

// doubleneq is a double equality test that does not use !=, instead
// it performs an approximate non-equality test for within
// 10·Epsilon.
static inline bool doubleneq(double a, double b) {
	return fabs(a - b) > 10 * Epsilon ||
		(std::isnan(a) != std::isnan(b)) ||
		(std::isinf(a) != std::isinf(b));
}

// between returns true if x is between min and max.
static inline bool between(double min, double max, double x) {
 	return x >= min - sqrt(Epsilon) && x <= max + sqrt(Epsilon);
}

// The Point structure represents a point in 2D space.
struct Point {

	// dot returns the dot product of the two points.
	static double dot(const Point &a, const Point &b) { return a.dot(b); }

	// sub returns the value of the component-wise subtraction
	// of b from a.
	static Point sub(const Point &a, const Point &b) { return a.minus(b); }

	// distance returns the Euclidean distance of two points.
	static double distance(const Point &a, const Point &b) {
		Point diff = b.minus(a);
		return sqrt(dot(diff, diff));
	}

	// inf returns a point with infinite values for both x and y.
	static Point inf(void) { return Point(Infinity, Infinity); }

	// neginf returns a point with negative infinity values for
	// both x and y.
	static Point neginf(void) { return Point(-Infinity, -Infinity); }

	// angle returns the angle to the point off of the positive x axis.
	// The value is between 0 and 2π.
	static double angle(const Point &a) {	return a.angle(); }

	// cwangle returns the clock-wise angle between the lines from
	// u to v and v to w.
	static double cwangle(const Point &u, const Point &v, const Point &w) {
		Point a = v.minus(u), b = w.minus(v);
		return -atan2(a.x*b.y - a.y*b.x, a.x*b.x+a.y*b.y);
	}

	Point(void) { }

	Point(const Point &o) : x(o.x), y(o.y) { }

	Point(double _x, double _y) : x(_x), y(_y) { }

	bool operator==(const Point &p) const {
		return doubleeq(x, p.x) && doubleeq(y, p.y);
	}

	bool operator!=(const Point &p) const { return !(*this == p); }

	void draw(Image &img, Color c = Image::black, double r = 1) const {
		img.add(new Image::Circle(x, y, r,  c, -1));
	}

	// angle returns the angle to the point off of the positive x axis.
	// The value is between 0 and 2π.
	double angle(void) const {
		double angle = atan2(y, x);
		if (angle < 0)
			return 2 * M_PI + angle;
		return angle;
	}

	// dot returns the dot product of this point with point b.
	double dot(const Point &b) const {
		return x * b.x + y * b.y;
	}

	// minus returns the point resulting from component-wise
	// subtraction of b from the current point.
	Point minus(const Point &b) const {
		return Point(x - b.x, y - b.y);
	}

	// move moves the point by the given deltas.
	void move(double dx, double dy) {
		x += dx;
		y += dy;
	}

	double x, y;
};

// The Line struct represents an infinite line in Euclidean space.
struct Line {
	Line(void) { }

	// Line(Point,Point) creates a line defined by the two points.
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

	// isection returns the point of intersection between two lines.
	// If the lines are parallel then Point::inf is returned.
	Point isection(const Line &l) const {
		if (std::isinf(m) || std::isinf(l.m))
			return vertisect(*this, l);

		if (doubleeq(m, l.m))
			return Point::inf();

		double x = (l.b - b) / (m - l.m);
		return Point(x, m * x + b);
	}

	// isabove returns true if the point is above the given line.
	bool isabove(const Point &p) const { return (m * p.x + b) < p.y; }

	// In case of a vertical line: m == ∞ and b = x
	double m, b;

private:
	static Point vertisect(const Line &a, const Line &b) {
		if (std::isinf(a.m) && std::isinf(b.m))
			return Point::inf();

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

	// length returns the length of the line segment.
	double length(void) const {
		return Point::distance(p0, p1);
	}

	// midpt returns the point directly in the middle of the
	// line segment.
	Point midpt(void) const {
		return Point((p0.x + p1.x) / 2, (p0.y + p1.y) / 2);
	}

	// along returns the point that is the given distance along
	// the line segment from the starting point.  The distance
	// can be greater than the length of the segment.
	Point along(double dist) const {
		double theta = Point::angle(p1.minus(p0));
		return Point(p0.x + dist * cos(theta), p0.y + dist * sin(theta));
	}

	// containt returns true if the line contains the given point.
	bool contains(const Point &p) const {
		if (doubleeq(m, 0.0))
			return doubleeq(mins.y, p.y) &&
				between(mins.x, maxes.x, p.x);
		return between(mins.x, maxes.x, p.x) &&
			between(mins.y, maxes.y, p.y);
	}

	// isection returns the point at which the two line segments
	// intersect.  If they do not intersect then Point::inf() is
	// returned.  If the Line (infinite line) corresponding to
	// the two lines is the same then the return value is
	// also Point::inf().
	Point isection(const LineSeg &l) const {
		if (isvertical() || l.isvertical())
			return vertisect(*this, l);

		Point p = Line::isection(l);
		if (!contains(p) || !l.contains(p))
			return Point::inf();
		return p;
	}

	// isvertical returns true if the line is a vertical line.
	bool isvertical(void) const { return doubleeq(p0.x, p1.x); }

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

struct Rectangle {
	Rectangle(void) : min(Point::inf()), max(Point::neginf()) { }

	Rectangle(double x0, double y0, double x1, double y1) :
		min(x0, y0), max(x1, y1) { normalize(); }

	Rectangle(const Point &_min, const Point &_max) :
			min(_min), max(_max) { normalize(); }

	// Get the bounding box for the set of points
	Rectangle(std::vector<Point> &pts) {
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

	bool operator==(const Rectangle &o) const {
		return min == o.min && max == o.max;
	}

	void draw(Image&, Color c = Image::black, double lwidth = 1) const;

	// contains returns true if the rectangle contains
	// the given point.
	bool contains(const Point &p) const {
		return between(min.x, max.x, p.x) && between(min.y, max.y, p.y);
	}

	// hits returns true if the rectangles intersect
	bool hits(const Rectangle &b) const {
		return !(min.x > b.max.x || b.min.x > max.x
			|| min.y > b.max.y || b.min.y > max.y);
	}

	// move translates the rectangle by the given
	// delta values.
	void move(double dx, double dy) {
		min.move(dx, dy);
		max.move(dx, dy);
	}

	Point min, max;

protected:

	void normalize(void) {
		if (min.x > max.x) {
			double t = min.x;
			min.x = max.x;
			max.x = t;
		}
		if (min.y > max.y) {
			double t = min.y;
			min.y = max.y;
			max.y = t;
		}
	}
};

struct Polygon {

	// random generates a random polygon for which no side
	// crosses the line between the vertices with the minimum
	// and maximum x value.
	static Polygon random(unsigned int n, double xc, double yc, double r);

	// giftwrap returns the polygon for the outter-hull of the
	// given set of points.
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
		bbox = Rectangle(verts);
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
	Rectangle bbox;

private:
	double interangle(unsigned int) const;
	bool isreflex(unsigned int) const;
	void removecolinear(void);
	void initsides(void);
};