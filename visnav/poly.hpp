#include "../utils/image.hpp"
#include <vector>

struct Point {
	Point(void) { }
	Point(double _x, double _y) : x(_x), y(_y) { }
	double x, y;
};

class Poly {
public:
	Poly(unsigned int nverts, ...);
	Poly(unsigned int nverts, va_list);
	Poly(std::vector<Point>&);

	static Poly triangle(double x, double y, double radius);
	static Poly square(double x, double y, double height);

	// If the width is <0 then the polygon is filled.
	void draw(Image&, Color, double width=1) const;
private:
	std::vector<Point> verts;
};