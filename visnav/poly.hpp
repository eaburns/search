#include "../utils/image.hpp"

struct Point {
	double x, y;
};

class Poly {
public:
	Poly(double xc, double yc, double radius, 
		unsigned int nsides, double rot);

	~Poly(void);

	// If the width is <0 then the polygon is filled.
	void draw(Image&, Color, double width=1) const;
private:
	void computepoints(void);

	double xc, yc, radius, innerangle, rotate;
	unsigned int nsides;
	Point *pts;
};