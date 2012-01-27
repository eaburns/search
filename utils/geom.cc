#include "geom.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdarg>
#include <cerrno>

static bool isisect(const Point&);
static unsigned int minx(const std::vector<Point>&);
static void xsortedpts(std::vector<Point>&, double, double, double);
static bool cmpx(const Point&, const Point&);

void Rectangle::draw(Image &img, Color c, double lwidth) const {
	Image::Path *p = new Image::Path();
	p->setlinejoin(Image::Path::Miter);
	p->setlinewidth(lwidth < 0 ? 0.1 : lwidth);
	p->setcolor(c);
	p->moveto(min.x, min.y);
	p->lineto(min.x, max.y);
	p->lineto(max.x, max.y);
	p->lineto(max.x, min.y);
	p->closepath();
	if (lwidth < 0)
		p->fill();
	img.add(p);
}

// vertices are given clock-wise around the polygon.
Polygon::Polygon(const std::vector<Point> &vs) : verts(vs), bbox(verts) {
	if (verts.size() < 3)
		fatal("A polygon needs at least 3 points\n");
	removecolinear();
	initsides();
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
	bbox = Rectangle(verts);
	removecolinear();
	initsides();
}

Polygon::Polygon(FILE *in) {
	unsigned int nverts;
	int res = fscanf(in, " %u", &nverts);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the polygon");
	if (res != 1)
		fatal("Malformed polygon");

	for (unsigned int i = 0; i < nverts; i++) {
		double x, y;
		res = fscanf(in, " %lg %lg", &x, &y);
		if (res == EOF)
			fatalx(errno, "Failed to read the polygon's vertices");
		if (res != 2)
			fatal("Malformed vertex");
		verts.push_back(Point(x, y));
	}
	bbox = Rectangle(verts);
	removecolinear();
	initsides();
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

Polygon Polygon::giftwrap(const std::vector<Point> &pts) {
	std::vector<Point> hull;
	unsigned int min = minx(pts);

	unsigned int cur = min, prev = pts.size();
	Point prevpt(pts[cur].x, pts[cur].y - 0.1);

	do {
		unsigned int next = 0;
		double ang = std::numeric_limits<double>::infinity();
		for (unsigned int i = 0; i < pts.size(); i++) {
			if (i == prev || i == cur)
				continue;
			double t = Point::cwangle(prevpt, pts[cur], pts[i]);
			if (t < ang) {
				ang = t;
				next = i;
			}
		}
		
		hull.push_back(pts[next]);
		prev = cur;
		prevpt = pts[prev];
		cur = next;
	} while (cur != min);

	return Polygon(hull);
}

void Polygon::output(FILE *out) const {
	fprintf(out, "%lu", (unsigned long) verts.size());
	for (unsigned int i = 0; i < verts.size(); i++)
		fprintf(out, " %g %g", verts[i].x, verts[i].y);
}

void Polygon::draw(Image &img, Color c, double lwidth) const {
	Image::Path *p = new Image::Path();
	p->setlinejoin(Image::Path::Round);
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

bool Polygon::contains(const Point &pt) const {
	bool even = true, isect = false;
	Line ray(pt, Point(pt.x + 1, pt.y));

	for (unsigned int i = 0; i < sides.size(); i++) {
		const LineSeg &side = sides[i];
		if (side.maxes.x < pt.x || side.maxes.y < pt.y || side.mins.y > pt.y)
			continue;
		Point hit = ray.isection(side);

		if (!isisect(hit) || hit.x <= pt.x || !side.contains(hit))
			continue;
		else if (side.p0 == hit && side.p1.y >= hit.y)
			continue;
		else if (side.p1 == hit && side.p0.y >= hit.y)
			continue;

		isect = true;
		even = !even;
	}

	return isect && !even;
}

std::vector<Point> Polygon::isections(const LineSeg &l) const {
	std::vector<Point> is;

	for (unsigned int i = 0; i < sides.size(); i++) {
		Point p = l.isection(sides[i]);
		if (isisect(p))
			is.push_back(p);
	}

	return is;
}

Point Polygon::minisect(const LineSeg &l) const {
	Point min = Point::inf();
	double mindist = std::numeric_limits<double>::infinity();

	for (unsigned int i = 0; i < sides.size(); i++) {
		Point p = l.isection(sides[i]);
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
		const LineSeg &side = sides[i];
		Point p = l.isection(side);
		if (isisect(p))
			return true;
	}
	return false;
}

double Polygon::interangle(unsigned int i) const {
	const Point &u = i == 0 ? verts[verts.size() - 1] : verts[i-1];
	const Point &v = verts[i];
	const Point &w = i == verts.size() - 1 ? verts[0] : verts[i+1];
	Point a = v.minus(u), b = w.minus(v);

	return M_PI - fmod(atan2(b.x*a.y - a.x*b.y, b.x*a.x + b.y*a.y), 2 * M_PI);
}

void Polygon::removecolinear(void) {
	for (unsigned int i = 0; i < verts.size(); ) {
		if (fabs(interangle(i) - M_PI) < std::numeric_limits<double>::epsilon())
			verts.erase(verts.begin() + i);
		else
			i++;
	}
}

void Polygon::initsides(void) {
	sides.clear();
	for (unsigned int i = 1; i < verts.size(); i++)
		sides.push_back(LineSeg(verts[i-1], verts[i]));
	sides.push_back(LineSeg(verts[verts.size() - 1], verts[0]));
}

static bool isisect(const Point &p) {
	return !std::isinf(p.x) && !std::isinf(p.y);
}

static unsigned int minx(const std::vector<Point> &pts) {
	unsigned int min = pts.size();
	for (unsigned int i = 0; i < pts.size(); i++) {
		if (min >= pts.size() || pts[i].x < pts[min].x)
			min = i;
	}
	return min;
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