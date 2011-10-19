#include "../utils/image.hpp"
#include <vector>

struct Point {
	Point(void) { }

	Point(double _x, double _y) : x(_x), y(_y) { }

	double x, y;
};

struct Line {
	Line(double _m, double _b) : m(_m), b(_b) { }

	Line(double x0, double y0, double x1, double y1) {
		init(x0, y0, x1, y1);
	}

	Line(Point a, Point b) { init(a.x, a.y, b.x, b.y); }

	bool above(Point &p) const { return above(p.x, p.y); }

	bool above(double x, double y) const {
		double liney = m * x + b;
		return liney < y;
	}

	double m, b;
private:
	void init(double x0, double y0, double x1, double y1);
};

class Poly {
public:
	Poly(unsigned int nverts, ...);

	Poly(unsigned int nverts, va_list);

	Poly(std::vector<Point> &_verts) : verts(_verts) { }

	static Poly random(unsigned int n, double xc, double yc, double r);

	static Poly triangle(double x, double y, double radius);

	static Poly square(double x, double y, double height);

	// If the width is <0 then the polygon is filled.
	void draw(Image&, Color, double width=1, bool number = false) const;

private:

	std::vector<Point> verts;
};
