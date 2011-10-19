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

	Point(void) { }

	Point(double _x, double _y) : x(_x), y(_y) { }

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

class Poly {
public:
	Poly(unsigned int nverts, ...);

	Poly(unsigned int nverts, va_list);

	Poly(std::vector<Point> &_verts) : verts(_verts) {
		computereflexes();
	}

	static Poly random(unsigned int n, double xc, double yc, double r);

	static Poly triangle(double x, double y, double radius);

	static Poly square(double x, double y, double height);

	// If the width is <0 then the polygon is filled.
	void draw(Image&, Color, double width=1, bool number = false) const;

	bool willhit(const Line&) const;

	double minhit(const Line&) const;

private:

	void computereflexes(void);

	// Interior angle of the ith vertex
	double interiorangle(unsigned int i) const;

	std::vector<Point> verts;
	// Reflex vertices are the ones which have interior
	// angles < π.
	std::vector<Point> reflexes;
};
