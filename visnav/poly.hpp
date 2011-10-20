#ifndef _POLY_HPP_
#define _POLY_HPP_

#include "../utils/image.hpp"
#include <vector>

struct Point {

	static double distance(const Point &p0, const Point &p1) {
		double dx = p1.x - p0.x, dy = p1.y - p0.y;
		return sqrt(dx * dx + dy * dy);
	}

	static Point subtract(const Point &a, const Point &b) {
		return (Point(b.x - a.x, b.y - a.y));
	}

	static Point scale(const Point &a, double sc) {
		return Point(a.x * sc, a.y * sc);
	}
 
	static double dot(const Point &a, const Point &b) {
		return a.x * b.x + a.y * b.y;
	}

	static Point normal(const Point &a) {
		double len = sqrt(a.x*a.x + a.y*a.y);
		return Point(a.x/len, a.y/len);
	}

	// between 0 and 2π.
	static double angle(const Point&, const Point&);

	// Angle from the positive x-axis.
	static double angle(const Point&);

	Point(void) { }

	Point(double _x, double _y) : x(_x), y(_y) { }

	void normalize(void) {
		double len = sqrt(x*x + y*y);
		x /= len;
		y /= len;
	}

	double x, y;
};

struct Line {

	static Point intersection(const Line&, const Line&);

	Line(double x0, double y0, double x1, double y1) :
			p0(x0, y0), p1(x1, y1) {
		init(x0, y0, x1, y1);
	}

	Line(const Point &a, const Point &b) : p0(a), p1(b) { init(a.x, a.y, b.x, b.y); }

	bool above(const Point &p) const { return above(p.x, p.y); }

	bool above(double x, double y) const {
		double liney = m * x + b;
		return liney < y;
	}

	Point midpoint(void) const {
		return Point((p0.x + p1.x) / 2, (p0.y + p1.y) / 2);
	}

	double length(void) const { return Point::distance(p0, p1); }

	bool contains(const Point &pt) const {
		return pt.x >= minx && pt.x <= maxx &&
			pt.y >= miny && pt.y <= maxy;
	};

	Point p0, p1;
	double m, b;
	double theta;	// angle from p0 to p1
	double minx, maxx, miny, maxy;
private:

	void init(double x0, double y0, double x1, double y1);
};

struct Poly {
	Poly(unsigned int nverts, ...);

	Poly(unsigned int nverts, va_list);

	Poly(std::vector<Point> &_verts) :
		verts(_verts), cntroid(FP_NAN, FP_NAN) { }

	static Poly random(unsigned int n, double xc, double yc, double r);

	static Poly triangle(double x, double y, double radius);

	static Poly square(double x, double y, double height);

	// If the width is <0 then the polygon is filled.
	void draw(Image&, Color, double width=1, bool number = false) const;

	bool contains(const Point &p) const;

	bool willhit(const Line&) const;

	// Ignore hits of distance < epsilon
	double minhit(const Line&, double epsilon = 0.0) const;

	bool isreflex(unsigned int i) const {
		return interiorangle(i) < M_PI;
	}

	const Point &centroid(void) {
		if (isnan(cntroid.x))
			computecentroid();
		return cntroid;
	}

	std::vector<Point> verts;
	Point cntroid;
private:

	void computecentroid(void);

	// Interior angle of the ith vertex
	double interiorangle(unsigned int i) const;
};

#endif	// _POLY_HPP_