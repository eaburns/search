#include "poly.hpp"
#include "../utils/image.hpp"
#include "../utils/utils.hpp"
#include <cmath>
#include <cstdarg>
#include <cassert>
#include <cstdio>

Poly::Poly(unsigned int nverts, ...) {
	va_list ap;
	va_start(ap, nverts);
	for (unsigned int i = 0; i < nverts; i++) {
		Point p;
		p.x = va_arg(ap, double);
		p.y = va_arg(ap, double);
		verts.push_back(p);
	}
	va_end(ap);
}

Poly::Poly(unsigned int nverts, va_list ap) {
	for (unsigned int i = 0; i < nverts; i++) {
		Point p;
		p.x = va_arg(ap, double);
		p.y = va_arg(ap, double);
		verts.push_back(p);
	}
}

Poly::Poly(std::vector<Point> &pts) : verts(pts){}

Poly Poly::triangle(double x, double y, double radius) {
	return Poly(3,
		x, y + radius,
		x + radius * cos(7 * M_PI / 6), y + radius * sin(7 * M_PI / 6),
		x + radius * cos(11 * M_PI / 6), y + radius * sin(11 * M_PI / 6));
}

Poly Poly::square(double x, double y, double height) {
	return Poly(4,
			x + height / 2, y + height / 2,
			x - height / 2, y + height / 2,
			x - height / 2, y - height / 2,
			x + height / 2, y - height / 2);
}

void Poly::draw(Image &img, Color c, double width) const {
	Image::Path *p = new Image::Path();
	p->setcolor(c);
	if (width >= 0)
		p->setlinewidth(width);
	else
		p->setlinewidth(0.1);
	p->moveto(verts[0].x, verts[0].y);
	for (unsigned int i = 1; i < verts.size(); i++)
		p->lineto(verts[i].x, verts[i].y);
	p->closepath();
	if (width < 0)
		p->fill();
	img.add(p);

	char buf[10];
	for (unsigned int i = 0; i < verts.size(); i++) {
		snprintf(buf, 10, "%u", i);
		img.add(new Image::Text(buf, verts[i].x, verts[i].y, 6));
	}
}
