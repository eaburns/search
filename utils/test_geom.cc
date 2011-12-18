#include "utils.hpp"
#include "geom.hpp"
#include <cstdarg>

static bool eq(double, double);
static void drawisects(const Polygon&, const LineSeg&);
static bool samepts(std::vector<Point>&, unsigned int n, ...);

bool test_point_angle(void) {
	struct { double x, y, theta; } tst[] = {
		{ 1, 0, 0 },
		{ 0, 1, M_PI / 2 },
		{ -1, 0, M_PI },
		{ 0, -1, 3 * M_PI / 2 },
		{ 1, 1, M_PI / 4 },
		{ -1, 1, 3 * M_PI / 4 },
		{ -1, -1, 5 * M_PI / 4 },
		{ 1, -1, 7 * M_PI / 4 },
	};
	bool ok = true;

	for (unsigned int i = 0; i < sizeof(tst) / sizeof(tst[0]); i++) {
		Point p(tst[i].x, tst[i].y);
		double t = p.angle();
		if (doubleneq(t, tst[i].theta)) {
			testpr("%g,%g, expected %grad, got %grad\n", tst[i].x,
				tst[i].y, tst[i].theta, t);
			ok = false;
		}
	}

	return ok;
}

bool test_line_isect(void) {
	struct { double x00, y00, x01, y01, x10, y10, x11, y11, xi, yi; } tst[] = {
		// same line
		{ 0, 0, 1, 0,	0, 0, 1, 0,	Infinity, Infinity },
		{ 0, 0, 1, 1,	0, 0, 1, 1,	Infinity, Infinity },
		{ 0, 0, 0, 1,	0, 0, 0, 1,	Infinity, Infinity },
		// parallel
		{ 0, 0, 1, 0,	0, 1, 1, 1,	Infinity, Infinity },
		{ 0, 0, 1, 1,	0, 1, 1, 2,	Infinity, Infinity },
		{ 0, 0, 0, 1,	1, 0, 1, 1,	Infinity, Infinity },
		// vertical/horizontal intersection
		{ 0, -1, 0, 1,	-1, 0, 1, 0,	0, 0 },
		// other intersections
		{ -1, -1, 1, 1,	-1, 1, 1, -1,	0, 0 },
		{ 0, 2, 2, 0,	0, 0, 2, 2,	1, 1 },
	};
	bool ok = true;

	for (unsigned int i = 0; i < sizeof(tst) / sizeof(tst[0]); i++) {
		Line l0(Point(tst[i].x00, tst[i].y00), Point(tst[i].x01, tst[i].y01));
		Line l1(Point(tst[i].x10, tst[i].y10), Point(tst[i].x11, tst[i].y11));
		Point expect(tst[i].xi, tst[i].yi);
		Point isect0 = l0.isection(l1);
		Point isect1 = l1.isection(l0);
		if (isect0 != isect1) {
			testpr("%g,%g → %g,%g; %g,%g → %g,%g: isect order differed\n",
				tst[i].x00, tst[i].y00, tst[i].x01, tst[i].y01,
				tst[i].x10, tst[i].y10, tst[i].x11, tst[i].y11);
			ok = false;
		}
		if (isect0 != expect) {
			testpr("%d: %g,%g → %g,%g; %g,%g → %g,%g: expected %g,%g, got %g,%g\n",
				i, tst[i].x00, tst[i].y00, tst[i].x01, tst[i].y01,
				tst[i].x10, tst[i].y10, tst[i].x11, tst[i].y11,
				expect.x, expect.y, isect0.x, isect0.y);
			ok = false;
		}
	}

	return ok;
}

bool test_line_isabove(void) {
	struct { double x0, y0, x1, y1, x, y; bool above; } tst[] = {
		{ -1, 0, 1, 0,	0, 1,	true },
		{ -1, 0, 1, 0,	0, -1,	false },
		// vertical line
		{ 0, -1, 0, 1,	0, 1,	false },
		{ 0, -1, 0, 1,	0, -1,	false },
		{ 0, -1, 0, 1,	0, 0,	false },
	};
	bool ok = true;

	for (unsigned int i = 0; i < sizeof(tst) / sizeof(tst[0]); i++) {
		Line l(Point(tst[i].x0, tst[i].y0), Point(tst[i].x1, tst[i].y1));
		Point pt(tst[i].x, tst[i].y);
		bool above = l.isabove(pt);
		if (above != tst[i].above) {
			testpr("%d: %g,%g → %g,%g; %g,%g expected above: %d, got %d\n",
				i, tst[i].x0, tst[i].y0, tst[i].x1, tst[i].y1,
				tst[i].x, tst[i].y, tst[i].above, above);
			ok = false;
		}
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