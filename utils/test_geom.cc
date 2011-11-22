#include "utils.hpp"
#include "geom.hpp"
#include <cstdarg>

static bool eq(double, double);
static void drawisects(const Polygon&, const LineSeg&);
static bool samepts(std::vector<Point>&, unsigned int n, ...);

bool test_point_angle(void) {
	bool ok = true;

	Point p(1, 0);
	double theta = Point::angle(p);
	if (!eq(theta, 0)) {
		testpr("Expected 1,0 to have angle 0, got %g\n", theta);
		ok = false;
	}

	p = Point(0, 1);
	theta = Point::angle(p);
	if (!eq(theta, M_PI / 2)) {
		testpr("Expected 0,1 to have angle π/2=%g, got %g\n",
			M_PI/2, theta);
		ok = false;
	}

	p = Point(-1, 0);
	theta = Point::angle(p);
	if (!eq(theta, M_PI)) {
		testpr("Expected -1,0 to have angle π=%g, got %g\n",
			M_PI, theta);
		ok = false;
	}

	p = Point(0, -1);
	theta = Point::angle(p);
	if (!eq(theta, 3 * M_PI / 2)) {
		testpr("Expected 0,-1 to have angle 3π/2=%g, got %g\n",
			3*M_PI/2, theta);
		ok = false;
	}

	return ok;
}

bool test_line_isect(void) {
	bool ok = true;

	Line a(Point(0, 0), Point(1, 1));
	Line b(Point(0, 0), Point(-1, 1));
	Point is = a.isection(b);
	if (!eq(is.x, 0) || !eq(is.y, 0)) {
		testpr("Expected isection 0,0, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = Line(Point(0, 0), Point(1, 1));
	b = Line(Point(0, 2), Point(2, 0));
	is = a.isection(b);
	if (!eq(is.x, 1) || !eq(is.y, 1)) {
		testpr("Expected isection 1,1, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = Line(Point(0, 0), Point(1, 1));
	b = Line(Point(0, 1), Point(1, 2));
	is = a.isection(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isection ∞,∞, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = Line(Point(0, 0), Point(1, 1));
	b = Line(Point(-1, -1), Point(2, 2));
	is = a.isection(b);
	if (!std::isnan(is.x) || !std::isnan(is.y)) {
		testpr("Expected isection NaN,NaN, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	return ok;
}

bool test_line_isabove(void) {
	bool ok = true;

	Line l(Point(0, 0), Point(1, 0));
	Point p(0, 1);
	if (!l.isabove(p)) {
		testpr("Expected above\n");
		ok = false;
	}
	p = Point(0, -1);
	if (l.isabove(p)) {
		testpr("Expected below\n");
		ok = false;
	}

	l = Line(Point(0, 0), Point(1, 1));
	p = Point(0, 2);
	if (!l.isabove(p)) {
		testpr("Expected above\n");
		ok = false;
	}
	p = Point(0, 0);
	if (l.isabove(p)) {
		testpr("Expected below\n");
		ok = false;
	}

	l = Line(Point(0, 0), Point(1, -1));
	p = Point(1, 0);
	if (!l.isabove(p)) {
		testpr("Expected above\n");
		ok = false;
	}
	p = Point(1, -2);
	if (l.isabove(p)) {
		testpr("Expected below\n");
		ok = false;
	}

	l = Line(Point(75.451, 112.008), Point(0.0, 45.0318));
	p = Point(52.0712, 56.7999);
	if (l.isabove(p)) {
		testpr("Expected %g,%g below %g,%g ↔ %g,%g\n",
			p.x, p.y, 75.451, 112.008, 0.0, 45.0318);
		ok = false;
	}

	return ok;
}

bool test_lineseg_along(void) {
	bool ok = true;
	Point origin(0,0);

	LineSeg l(origin, Point(1, 0));
	Point a = l.along(10);
	if (!eq(a.x, 10) || !eq(a.y, 0)) {
		testpr("Expect 10,0, got %g,%g\n", a.x, a.y);
		ok = false;
	}
	a = l.along(-10);
	if (!eq(a.x, -10) || !eq(a.y, 0)) {
		testpr("Expect -10,0, got %g,%g\n", a.x, a.y);
		ok = false;
	}

	l = LineSeg(origin, Point(-1, 0));
	a = l.along(10);
	if (!eq(a.x, -10) || !eq(a.y, 0)) {
		testpr("Expect -10,0, got %g,%g\n", a.x, a.y);
		ok = false;
	}
	a = l.along(-10);
	if (!eq(a.x, 10) || !eq(a.y, 0)) {
		testpr("Expect 10,0, got %g,%g\n", a.x, a.y);
		ok = false;
	}

	l = LineSeg(origin, Point(0, 1));
	a = l.along(10);
	if (!eq(a.x, 0) || !eq(a.y, 10)) {
		testpr("Expect 0,10, got %g,%g\n", a.x, a.y);
		ok = false;
	}
	a = l.along(-10);
	if (!eq(a.x, 0) || !eq(a.y, -10)) {
		testpr("Expect 0,-10, got %g,%g\n", a.x, a.y);
		ok = false;
	}

	l = LineSeg(origin, Point(0, -1));
	a = l.along(10);
	if (!eq(a.x, 0) || !eq(a.y, -10)) {
		testpr("Expect 0,-10, got %g,%g\n", a.x, a.y);
		ok = false;
	}
	a = l.along(-10);
	if (!eq(a.x, 0) || !eq(a.y, 10)) {
		testpr("Expect 0,10, got %g,%g\n", a.x, a.y);
		ok = false;
	}

	return ok;
}

bool test_lineseg_midpt(void) {
	bool ok = true;

	LineSeg l(Point(0, 0), Point(1, 0));
	Point p = l.midpt();
	if (!eq(p.x, 0.5) || !eq(p.y, 0)) {
		testpr("Expected ½,0, got %g,%g\n", p.x, p.y);
		ok = false;
	}

	l = LineSeg(Point(0, 0), Point(-1, 0));
	p = l.midpt();
	if (!eq(p.x, -0.5) || !eq(p.y, 0)) {
		testpr("Expected -½,0, got %g,%g\n", p.x, p.y);
		ok = false;
	}

	l = LineSeg(Point(0, 0), Point(0, 1));
	p = l.midpt();
	if (!eq(p.x, 0) || !eq(p.y, 0.5)) {
		testpr("Expected 0,½, got %g,%g\n", p.x, p.y);
		ok = false;
	}

	l = LineSeg(Point(0, 0), Point(0, -1));
	p = l.midpt();
	if (!eq(p.x, 0) || !eq(p.y, -0.5)) {
		testpr("Expected 0,-½, got %g,%g\n", p.x, p.y);
		ok = false;
	}

	return ok;
}

bool test_lineseg_length(void) {
	bool ok = true;

	LineSeg l(Point(0, 0), Point(1, 0));
	if (!eq(l.length(), 1)) {
		testpr("Expected length 1, got %g\n", l.length());
		ok = false;
	}

	l = LineSeg(Point(0, 0), Point(-1, 0));
	if (!eq(l.length(), 1)) {
		testpr("Expected length 1, got %g\n", l.length());
		ok = false;
	}

	l = LineSeg(Point(0, 0), Point(0, 1));
	if (!eq(l.length(), 1)) {
		testpr("Expected length 1, got %g\n", l.length());
		ok = false;
	}

	l = LineSeg(Point(0, 0), Point(0, -1));
	if (!eq(l.length(), 1)) {
		testpr("Expected length 1, got %g\n", l.length());
		ok = false;
	}

	l = LineSeg(Point(1, 1), Point(2, 0));
	if (!eq(l.length(), sqrt(2))) {
		testpr("Expected length √2=%g, got %g\n", sqrt(2), l.length());
		ok = false;
	}

	return ok;
}

bool test_lineseg_contains(void) {
	bool ok = true;

	LineSeg l(Point(-1, 0), Point(1, 0));
	if (!l.contains(Point(0, 0))) {
		testpr("Expected 0,0 to be contained in -1,0 → 1.0\n");
		ok = false;
	}

	l = LineSeg(Point(0, -1), Point(0, 1));
	if (!l.contains(Point(0, 0))) {
		testpr("Expected 0,0 to be contained in 0,-1 → 0,1\n");
		ok = false;
	}

	return ok;
}

bool test_lineseg_isect(void) {
	bool ok = true;

	LineSeg a(Point(-1, 0), Point(1, 0));
	LineSeg b(Point(0, -1), Point(0, 1));
	Point is = a.isection(b);
	if (!eq(is.x, 0) || !eq(is.y, 0)) {
		testpr("Expected isection 0,0, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(0, -1), Point(0, 1));
	b = LineSeg(Point(1, 0), Point(-1, 0));
	is = a.isection(b);
	if (!eq(is.x, 0) || !eq(is.y, 0)) {
		testpr("Expected isection 0,0, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(0, -1), Point(0, 1));
	b = LineSeg(Point(1, -1), Point(1, 1));
	is = a.isection(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isection ∞,∞, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(-1, 0), Point(1, 0));
	b = LineSeg(Point(-1, 1), Point(1, 1));
	is = a.isection(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isection ∞,∞, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(-1, 0), Point(1, 0));
	b = LineSeg(Point(-1, -2), Point(1, -1));
	is = a.isection(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isection ∞,∞, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(-1, -1), Point(1, 1));
	b = LineSeg(Point(-1, 1), Point(1, -1));
	is = a.isection(b);
	if (!eq(is.x, 0) || !eq(is.y, 0)) {
		testpr("Expected isection 0,0, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(0, 0), Point(2, 2));
	b = LineSeg(Point(0, 2), Point(2, 0));
	is = a.isection(b);
	if (!eq(is.x, 1) || !eq(is.y, 1)) {
		testpr("Expected isection 1,1, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(0, 0), Point(0, 1));
	b = LineSeg(Point(0.5, 0.5), Point(1.5, 0.5));
	is = a.isection(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isect of 0,0 → 0,1 with ½,½ → 1½,1½ to be ∞,∞, got %g,%g\n",
			is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(0, 1), Point(1, 1));
	b = LineSeg(Point(0.5, 0.5), Point(1.5, 0.5));
	is = a.isection(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isect of 0,1 → 1,1 with ½,½ → 1½,1½ to be ∞,∞, got %g,%g\n",
			is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(1, 1), Point(1, 0));
	b = LineSeg(Point(0.5, 0.5), Point(1.5, 0.5));
	is = a.isection(b);
	if (!eq(is.x, 1) || !eq(is.y, 0.5)) {
		testpr("Expected isect of 1,1 → 1,0 with ½,½ → 1½,1½ to be 1,½, got %g,%g\n",
			is.x, is.y);
		ok = false;
	}

	return ok;
}

bool test_poly_contains(void) {
	bool ok = true;

	Polygon sq(4,
		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0,
		0.0, 1.0);

	Point p(0.5, 0.5);
	if (!sq.contains(p)) {
		testpr("Expected ½,½ to be contained\n");
		ok = false;
	}

	p = Point(2, 2);
	if (sq.contains(p)) {
		testpr("Expected 2,2 not to be contained\n");
		ok = false;
	}

	Polygon poly(7, 0.0, 45.0318,
				52.0712, 56.7999, 
				59.1066, 109.825,
				61.2547, 87.3769,
				75.451, 112.008,
				85.8799, 15.2566,
				22.9319, 0.0);
	p = LineSeg(Point(0, 45.0318), Point(85.8799, 15.2566)).midpt();
	if (!poly.contains(p)) {
		testpr("Expected %g,%g to be contained\n", p.x, p.y);
		ok = false;
	}

	return ok;
}

bool test_poly_isects(void) {
	bool ok = true;

	std::vector<Point> is;
	Polygon sq(4, 0.0, 0.0, 100.0, 0.0, 100.0, 100.0, 0.0, 100.0);

	LineSeg l(Point(50, 50), Point(150, 50));
	is = sq.isections(l);
	if (!samepts(is, 1, 100.0, 50.0)) {
		testpr("Unexpected collisions for 50,50 → 150,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(-50, 50), Point(150, 50));
	is = sq.isections(l);
	if (!samepts(is, 2, 0.0, 50.0, 100.0, 50.0)) {
		testpr("Unexpected collisions for -50,50 → 150,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(150, 50), Point(250, 50));
	is = sq.isections(l);
	if (!samepts(is, 0)) {
		testpr("Unexpected collisions for 150,50 → 250,50: ");
		drawisects(sq, l);
		ok = false;
	}

	return ok;
}

bool test_poly_minisect(void) {
	bool ok = true;

	Polygon sq(4, 0.0, 0.0, 100.0, 0.0, 100.0, 100.0, 0.0, 100.0);

	LineSeg l(Point(50, 50), Point(150, 50));
	Point m = sq.minisect(l);
	if (!eq(m.x, 100) || !eq(m.y, 50)) {
		testpr("Unexpected min intersection for 50,50 → 150,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(-50, 50), Point(150, 50));
	m = sq.minisect(l);
	if (!eq(m.x, 0) || !eq(m.y, 50)) {
		testpr("Unexpected min intersection for -50,50 → 150,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(150, 50), Point(-50, 50));
	m = sq.minisect(l);
	if (!eq(m.x, 100) || !eq(m.y, 50)) {
		testpr("Unexpected min intersection for 150,50 → -50,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(150, 50), Point(250, 50));
	m = sq.minisect(l);
	if (!std::isinf(m.x) || !std::isinf(m.y)) {
		testpr("Unexpected min intersection for 150,50 → 250,50: ");
		drawisects(sq, l);
		ok = false;
	}

	return ok;
}

bool test_poly_hits(void) {
	bool ok = true;

	Polygon sq(4, 0.0, 0.0, 100.0, 0.0, 100.0, 100.0, 0.0, 100.0);

	LineSeg l(Point(50, 50), Point(150, 50));
	if (!sq.hits(l)) {
		testpr("Unexpected miss with 50,50 → 150,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(-50, 50), Point(150, 50));
	if (!sq.hits(l)) {
		testpr("Unexpected miss with -50,50 → 150,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(150, 50), Point(-50, 50));
	if (!sq.hits(l)) {
		testpr("Unexpected miss with 150,50 → -50,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(150, 50), Point(250, 50));
	if (sq.hits(l)) {
		testpr("Unexpected hit with 150,50 → 250,50: ");
		drawisects(sq, l);
		ok = false;
	}

	return ok;
}

static bool eq(double x, double y) {
	return fabs(x - y) < 1e-5;
}

enum { Bufsz = 128 };

static void drawisects(const Polygon &poly, const LineSeg &line) {
	static unsigned int n = 0;
	double w = poly.bbox.max.x - poly.bbox.min.x;
	double h = poly.bbox.max.y - poly.bbox.min.y;

	Image img(w, h, "failure.eps");

	poly.draw(img, Image::green);
	line.draw(img, Image::red);

	std::vector<Point> is = poly.isections(line);
	for (unsigned int i = 0; i < is.size(); i++)
		is[i].draw(img, Image::black, 4);

	poly.minisect(line).draw(img, Image::red, 2);

	char buf[Bufsz];
	snprintf(buf, Bufsz, "failure%u.eps", n++);
	testpr("saved in %s\n", buf);
	img.save(buf, true);
}

static bool samepts(std::vector<Point> &pts, unsigned int n, ...) {
	va_list ap;

	if (pts.size() != n)
		return false;

	va_start(ap, n);
	for (unsigned int i = 0; i < n; i++) {
		double x = va_arg(ap, double);
		double y = va_arg(ap, double);

		bool found = false;
		for (unsigned int j = 0; j < pts.size(); j++) {
			if (eq(pts[j].x, x) && eq(pts[j].y, y)) {
				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}
	va_end(ap);

	return true;
}