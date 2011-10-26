#include "geom.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdarg>

static void xsortedpts(std::vector<Point>&, double, double, double);
static bool cmpx(const Point&, const Point&);

void Bbox::draw(Image &img, Color c, double lwidth) const {
	Image::Path *p = new Image::Path();
	p->setlinewidth(lwidth < 0 ? 0.1 : lwidth);
	p->setcolor(c);
	p->moveto(min.x, min.y);
	p->lineto(min.x, max.y);
	p->lineto(max.x, max.y);
	p->lineto(max.x, min.y);
	p->closepath();
	img.add(p);
}

Polygon::Polygon(unsigned int n, ...) {
	va_list ap;
	va_start(ap, n);
	for (unsigned int i = 0; i < n; i++) {
		double x = va_arg(ap, double);
		double y = va_arg(ap, double);
		verts.push_back(Point(x, y));
	}
	va_end(ap);
	bbox = Bbox(verts);
	initsides(verts);
}

void Polygon::output(FILE *out) const {
	fprintf(out, "%lu", (unsigned long) verts.size());
	for (unsigned int i = 0; i < verts.size(); i++)
		fprintf(out, " %g %g", verts[i].x, verts[i].y);
	fputc('\n', out);
}

static bool isisect(const Point &p) {
	return !std::isinf(p.x) && !std::isnan(p.x) && !std::isinf(p.y) && !std::isnan(p.y);
}

bool Polygon::contains(const Point &p) const {
	bool even = true, isect = false;
	Line ray(p, Point(p.x + 1, p.y));

	for (unsigned int i = 0; i < sides.size(); i++) {
		Point hit = ray.isect(sides[i]);
		if (isisect(hit) && hit.x > p.x && sides[i].contains(hit)) {
			isect = true;
			even = !even;
		}
	}

	return isect && !even;
}

void Polygon::isects(const LineSeg &l, std::vector<Point> &is) const {
	is.clear();
	for (unsigned int i = 0; i < sides.size(); i++) {
		Point p = l.isect(sides[i]);
		if (isisect(p))
			is.push_back(p);
	}
}

Point Polygon::minisect(const LineSeg &l) const {
	Point min = Point::inf();
	double mindist = std::numeric_limits<double>::infinity();

	for (unsigned int i = 0; i < sides.size(); i++) {
		Point p = l.isect(sides[i]);
		double d = Point::distance(p, l.p0);
		if (isisect(p) && d < mindist) {
			mindist = d;
			min = p;
		}
	}

	return min;
}

bool Polygon::hits(const LineSeg &l) const {
	for (unsigned int i = 0; i < sides.size(); i++) {
		Point p = l.isect(sides[i]);
		if (isisect(p))
			return true;
	}
	return false;
}

void Polygon::draw(Image &img, Color c, double lwidth) const {
	Image::Path *p = new Image::Path();
	p->setlinewidth(lwidth < 0 ? 0.1 : lwidth);
	p->setcolor(c);
	p->moveto(verts[0].x, verts[0].y);
	for (unsigned int i = 1; i < verts.size(); i++)
		p->lineto(verts[i].x, verts[i].y);
	p->closepath();
	if (lwidth < 0)
		p->fill();
	img.add(p);
}

void Polygon::initsides(std::vector<Point> &pts) {
	sides.clear();
	for (unsigned int i = 1; i < pts.size(); i++)
		sides.push_back(LineSeg(pts[i-1], pts[i]));
	sides.push_back(LineSeg(pts[pts.size() - 1], pts[0]));
}

Polygon Polygon::random(unsigned int n, double xc, double yc, double r) {
	if (n < 3)
		fatal("A polygon needs at least 3 points\n");

	std::vector<Point> sorted(n);
	xsortedpts(sorted, xc, yc, r);

	Line l(sorted[0], sorted[sorted.size()-1]);
	std::vector<Point> verts, rev;

	verts.push_back(sorted[0]);
	for (unsigned int i = 1; i < sorted.size() - 1; i++) {
		assert (sorted[i].x > sorted[i-1].x);
		if (l.isabove(sorted[i]))
			verts.push_back(sorted[i]);
		else
			rev.push_back(sorted[i]);
	}

	verts.push_back(sorted[sorted.size() - 1]);

	while (rev.size() > 0) {
		assert (rev.back().x < verts[verts.size()-1].x);
		verts.push_back(rev.back());
		rev.pop_back();
	}

	return Polygon(verts);
}

void Polygon::reflexes(std::vector<unsigned int> &rs) const {
	rs.clear();

	for (unsigned int i = 0; i < verts.size(); i++) {
		const Point &u = i == 0 ? verts[verts.size() - 1] : verts[i-1];
		const Point &v = verts[i];
		const Point &w = i == verts.size() - 1 ? verts[0] : verts[i+1];
		Point a = v.minus(u), b = w.minus(v);

		// interior angle via some voodoo from the internet.
		double t = M_PI - fmod(atan2(b.x*a.y - a.x*b.y, b.x*a.x + b.y*a.y), 2 * M_PI);

		if (t < M_PI)
			rs.push_back(i);
	}
}

static void xsortedpts(std::vector<Point> &pts, double xc, double yc, double r) {
	for (unsigned int i = 0; i < pts.size(); i++) {
		double x = (2 * r * randgen.real()) - r;
		double y = (2 * r * randgen.real()) - r;
		pts[i] = Point(x + xc, y + yc);
	}

	std::sort(pts.begin(), pts.end(), cmpx);
}

static bool cmpx(const Point &a, const Point &b) {
	return a.x < b.x;
}