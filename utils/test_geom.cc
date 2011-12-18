#include "utils.hpp"
#include "geom.hpp"
#include <cstdarg>

static void drawisects(const Polygon&, const LineSeg&);
static bool samepts(std::vector<Point>&, unsigned int n, ...);

static struct { double a, b; bool eq; } eqtst[] = {
	{ 1, 1, true },
	{ 2, 2, true },
	{ 1e90, 1e90, true },
	{ M_PI, M_PI, true },
	{ 1, 2, false },
	{ 2, 3, false },
	{ 1e90, 1.0000001e90, false },
	{ M_PI, M_PI + 11 * Epsilon, false },
};

bool test_doubleeq(void) {
	bool ok = true;
	for (unsigned int i = 0; i < sizeof(eqtst) / sizeof(eqtst[0]); i++) {
		if (doubleneq(eqtst[i].a, eqtst[i].b) != eqtst[i].eq)
			continue;

		const char *expect = eqtst[i].eq ? "true" : "false";
		const char *got = eqtst[i].eq ? "false" : "true";
		testpr("expected doubleeq(%g,%g)=%s, but got %s\n",
			eqtst[i].a, eqtst[i].b, expect, got);
		ok = false;
	}
	return ok;
}

bool test_doubleneq(void) {
	bool ok = true;
	for (unsigned int i = 0; i < sizeof(eqtst) / sizeof(eqtst[0]); i++) {
		if (doubleeq(eqtst[i].a, eqtst[i].b) == eqtst[i].eq)
			continue;

		const char *expect = eqtst[i].eq ? "true" : "false";
		const char *got = eqtst[i].eq ? "false" : "true";
		testpr("expected doubleeq(%g,%g)=%s, but got %s\n",
			eqtst[i].a, eqtst[i].b, expect, got);
		ok = false;
	}
	return ok;
}

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
		if (doubleeq(t, tst[i].theta))
			continue;
		testpr("%g,%g, expected %g rad, got %g rad\n", tst[i].x,
			tst[i].y, tst[i].theta, t);
		ok = false;
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
		if (isect0 == expect)
			continue;
		testpr("%d: %g,%g → %g,%g; %g,%g → %g,%g: expected %g,%g, got %g,%g\n",
			i, tst[i].x00, tst[i].y00, tst[i].x01, tst[i].y01,
			tst[i].x10, tst[i].y10, tst[i].x11, tst[i].y11,
			expect.x, expect.y, isect0.x, isect0.y);
		ok = false;
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
		if (above == tst[i].above)
			continue;
		testpr("%d: %g,%g → %g,%g; %g,%g expected above: %d, got %d\n",
			i, tst[i].x0, tst[i].y0, tst[i].x1, tst[i].y1,
			tst[i].x, tst[i].y, tst[i].above, above);
		ok = false;
	}

	return ok;
}

bool test_lineseg_length(void) {
	struct { double x0, y0, x1, y1, len; } tst[] = {
		{ 0, 0, 1, 0,	1 },
		{ 0, 0, -1, 0,	1 },
		{ 0, 0, 0, 1,	1 },
		{ 0, 0, 0, -1,	1 },
		{ 0, 0, M_PI, 0,	M_PI },
		{ 0, 0, -M_PI, 0,	M_PI },
		{ 0, 0, 0, M_PI,	M_PI },
		{ 0, 0, 0, -M_PI,	M_PI },
		{ 0, 0, 1, 1,	sqrt(2) },
		{ 0, 0, 3, 4,	5 },
	};
	bool ok = true;

	for (unsigned int i = 0; i < sizeof(tst) / sizeof(tst[0]); i++) {
		LineSeg l(Point(tst[i].x0, tst[i].y0), Point(tst[i].x1, tst[i].y1));
		double len = l.length();
		if (doubleeq(len, tst[i].len))
			continue;
		testpr("%d: %g,%g → %g,%g expected length: %g, got %g\n",
			i, tst[i].x0, tst[i].y0, tst[i].x1, tst[i].y1,
			tst[i].len, len);
		ok = false;
	}

	return ok;
}

bool test_lineseg_midpt(void) {
	struct { double x0, y0, x1, y1, x, y; } tst[] = {
		{ -1, 0, 1, 0,	0, 0 },
		{ 1, 0, -1, 0,	0, 0 },
		{ 0, -1, 0, 1,	0, 0 },
		{ 0, 1, 0, -1,	0, 0 },
		{ -1, -1, 1, 1,	0, 0 },
		{ -1, 1, 1, -1,	0, 0 },
		{ 1e8, 1e8, 2e8, 2e8,	1.5e8, 1.5e8 },
	};
	bool ok = true;

	for (unsigned int i = 0; i < sizeof(tst) / sizeof(tst[0]); i++) {
		LineSeg l(Point(tst[i].x0, tst[i].y0), Point(tst[i].x1, tst[i].y1));
		Point mid(l.midpt());
		if (mid == Point(tst[i].x, tst[i].y))
			continue;
		testpr("%d: %g,%g → %g,%g expected length: %g,%g, got %g,%g\n",
			i, tst[i].x0, tst[i].y0, tst[i].x1, tst[i].y1,
			tst[i].x, tst[i].y, mid.x, mid.y);
		ok = false;
	}

	return ok;
}


bool test_lineseg_along(void) {
	struct { double x0, y0, x1, y1, dist, x, y; } tst[] = {
		{ 0, 0, 1, 0,	0.5,	 0.5, 0 },
		{ 0, 0, 0, 1,	0.5,	 0, 0.5 },
		{ 0, 0, 1, 1,	0.5,	 0.5*cos(M_PI/4), 0.5*sin(M_PI/4) },
		// greater than the length of the segment
		{ 0, 0, 1, 0,	10,	 10, 0 },
		{ 0, 0, 0, 1,	10,	 0, 10 },
		{ 0, 0, 1, 1,	10,	 10*cos(M_PI/4), 10*sin(M_PI/4) },
		// negative
		{ 0, 0, 1, 0,	-10,	 -10, 0 },
		{ 0, 0, 0, 1,	-10,	 0, -10 },
		{ 0, 0, 1, 1,	-10,	 -10*cos(M_PI/4), -10*sin(M_PI/4) },
	
	};
	bool ok = true;

	for (unsigned int i = 0; i < sizeof(tst) / sizeof(tst[0]); i++) {
		LineSeg l(Point(tst[i].x0, tst[i].y0), Point(tst[i].x1, tst[i].y1));
		Point expect(tst[i].x, tst[i].y);
		Point pt(l.along(tst[i].dist));
		if (pt == expect)
			continue;
		testpr("%d: %g,%g → %g,%g; %g along, expected %g,%g got %g,%g\n",
			i, tst[i].x0, tst[i].y0, tst[i].x1, tst[i].y1,
			tst[i].dist, tst[i].x, tst[i].y, pt.x, pt.y);
		ok = false;
	}

	return ok;
}

bool test_lineseg_contains(void) {
	struct { double x0, y0, x1, y1, x, y; bool cont; } tst[] = {
		{ 0, 0, 1, 0,	0, 0,	true },
		{ 0, 0, 1, 0,	1, 0,	true },
		{ -1, 0, 0, 0,	0, 0,	true },
		{ -1, 0, 0, 0,	-1, 0,	true },
		{ -1, 0, 1, 0,	0, 0,	true },
		{ 0, -1, 0, 1,	0, 0,	true },
		{ -1, -1, 1, 1,	0, 0,	true },
		{ 1, -1, -1, 1,	0, 0,	true },
		{ 0, 0, 5, 0,	M_PI, 0,	true },
		{ 0, 0, 5, 0,	6, 0,	false },
		{ -1, 0, 1, 0,	0, 1,	false },
		{ 0, -1, 0, 1,	1, 0,	false },
		{ 0, 0, 5, 0,	M_PI, 1,	false },
	
	};
	bool ok = true;

	for (unsigned int i = 0; i < sizeof(tst) / sizeof(tst[0]); i++) {
		LineSeg l(Point(tst[i].x0, tst[i].y0), Point(tst[i].x1, tst[i].y1));
		if (l.contains(Point(tst[i].x, tst[i].y)) == tst[i].cont)
			continue;
		testpr("%d: %g,%g → %g,%g;  %g,%g expected %d\n",
			i, tst[i].x0, tst[i].y0, tst[i].x1, tst[i].y1,
			tst[i].x, tst[i].y, tst[i].cont);
		ok = false;
	}

	return ok;
}

bool test_lineseg_isect(void) {
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
		// end-on-end (same Line, so Point::inf())
		{ 0, 0, 1, 0,	-1, 0, 0, 0,	Infinity, Infinity },
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
		if (isect0 == expect)
			continue;
		testpr("%d: %g,%g → %g,%g; %g,%g → %g,%g: expected %g,%g, got %g,%g\n",
			i, tst[i].x00, tst[i].y00, tst[i].x01, tst[i].y01,
			tst[i].x10, tst[i].y10, tst[i].x11, tst[i].y11,
			expect.x, expect.y, isect0.x, isect0.y);
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
	if (doubleneq(m.x, 100) || doubleneq(m.y, 50)) {
		testpr("Unexpected min intersection for 50,50 → 150,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(-50, 50), Point(150, 50));
	m = sq.minisect(l);
	if (doubleneq(m.x, 0) || doubleneq(m.y, 50)) {
		testpr("Unexpected min intersection for -50,50 → 150,50: ");
		drawisects(sq, l);
		ok = false;
	}

	l = LineSeg(Point(150, 50), Point(-50, 50));
	m = sq.minisect(l);
	if (doubleneq(m.x, 100) || doubleneq(m.y, 50)) {
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
			if (doubleeq(pts[j].x, x) && doubleeq(pts[j].y, y)) {
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