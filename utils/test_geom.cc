#include "utils.hpp"
#include "geom.hpp"

bool eq(double x, double y) {
	return fabs(x - y) < 1e-5;
}

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
	Point is = a.isect(b);
	if (!eq(is.x, 0) || !eq(is.y, 0)) {
		testpr("Expected isection 0,0, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = Line(Point(0, 0), Point(1, 1));
	b = Line(Point(0, 2), Point(2, 0));
	is = a.isect(b);
	if (!eq(is.x, 1) || !eq(is.y, 1)) {
		testpr("Expected isection 1,1, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = Line(Point(0, 0), Point(1, 1));
	b = Line(Point(0, 1), Point(1, 2));
	is = a.isect(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isection ∞,∞, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = Line(Point(0, 0), Point(1, 1));
	b = Line(Point(-1, -1), Point(2, 2));
	is = a.isect(b);
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
		testpr("Expected 0,0 to be contained in -1,0 -> 1.0\n");
		ok = false;
	}

	l = LineSeg(Point(0, -1), Point(0, 1));
	if (!l.contains(Point(0, 0))) {
		testpr("Expected 0,0 to be contained in 0,-1 -> 0,1\n");
		ok = false;
	}

	return ok;
}

bool test_lineseg_isect(void) {
	bool ok = true;

	LineSeg a(Point(-1, 0), Point(1, 0));
	LineSeg b(Point(0, -1), Point(0, 1));
	Point is = a.isect(b);
	if (!eq(is.x, 0) || !eq(is.y, 0)) {
		testpr("Expected isection 0,0, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(0, -1), Point(0, 1));
	b = LineSeg(Point(1, 0), Point(-1, 0));
	is = a.isect(b);
	if (!eq(is.x, 0) || !eq(is.y, 0)) {
		testpr("Expected isection 0,0, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(0, -1), Point(0, 1));
	b = LineSeg(Point(1, -1), Point(1, 1));
	is = a.isect(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isection ∞,∞, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(-1, 0), Point(1, 0));
	b = LineSeg(Point(-1, 1), Point(1, 1));
	is = a.isect(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isection ∞,∞, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(-1, 0), Point(1, 0));
	b = LineSeg(Point(-1, -2), Point(1, -1));
	is = a.isect(b);
	if (!std::isinf(is.x) || !std::isinf(is.y)) {
		testpr("Expected isection ∞,∞, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(-1, -1), Point(1, 1));
	b = LineSeg(Point(-1, 1), Point(1, -1));
	is = a.isect(b);
	if (!eq(is.x, 0) || !eq(is.x, 0)) {
		testpr("Expected isection 0,0, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	a = LineSeg(Point(0, 0), Point(2, 2));
	b = LineSeg(Point(0, 2), Point(2, 0));
	is = a.isect(b);
	if (!eq(is.x, 1) || !eq(is.x, 1)) {
		testpr("Expected isection 1,1, got %g,%g\n", is.x, is.y);
		ok = false;
	}

	return ok;
}