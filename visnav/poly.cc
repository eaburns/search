#include "poly.hpp"
#include "../utils/image.hpp"
#include "../utils/utils.hpp"
#include <cmath>

Poly::Poly(double _xc, double _yc, double _radius, unsigned int _nsides,
		double _rotate) :
		xc(_xc), yc(_yc), radius(_radius), rotate(_rotate),
		nsides(_nsides) {
	if (nsides < 3)
		fatal("A polygon must have at least 3 sides\n");
	innerangle = 2 * M_PI / nsides;
	computepoints();
}

Poly::~Poly(void) {
	delete[] pts;
}

void Poly::draw(Image &img, Color c, double width) const {
	Image::Path *p = new Image::Path();
	p->setcolor(c);
	if (width >= 0)
		p->setlinewidth(width);
	else
		p->setlinewidth(0.1);
	p->moveto(pts[0].x, pts[0].y);
	for (unsigned int i = 1; i < nsides; i++)
		p->lineto(pts[i].x, pts[i].y);
	p->closepath();
	if (width < 0)
		p->fill();
	img.add(p);
}

void Poly::computepoints(void) {
	pts = new Point[nsides];

	for (unsigned int i = 0; i < nsides; i++) {
		double angle = rotate + (innerangle * i);
		pts[i].x = xc + radius * cos(angle);
		pts[i].y = yc + radius * sin(angle);
	}
}
