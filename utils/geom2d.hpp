#ifndef _GEOM2D_HPP_
#define _GEOM2D_HPP_

#include <iterator>
#include <vector>
#include <limits>
#include <cmath>
#include <cstdio>
#include <cassert>

extern void fatal(const char*, ...);	// from utils.hpp

namespace Geom2d {
	
	// Epsilon is the smallest double value that can be added
	// to 1 to make it no longer equal to 1.
	static const double Epsilon = std::numeric_limits<double>::epsilon();
	
	// Inifinity is the double representation of inifinity.
	static const double Infinity = std::numeric_limits<double>::infinity();
	
	// Threshold is the threshold to which equality is performed
	static const double Threshold = sqrt(Epsilon);
	
	// doubleeq is a double equality test that does not use ==, instead
	// it performs an approximate equality test for within
	// 1e-10.
	static inline bool doubleeq(double a, double b) {
		return fabs(a - b) < Threshold || (std::isinf(a) && std::isinf(b));
	}
	
	// doubleneq is a double equality test that does not use !=, instead
	// it performs an approximate non-equality test for within
	// 1e-10.
	static inline bool doubleneq(double a, double b) {
		return fabs(a - b) > Threshold || (std::isinf(a) != std::isinf(b));
	}
	
	// between returns true if x is between min and max.
	static inline bool between(double min, double max, double x) {
	 	return x >= min - sqrt(Epsilon) && x <= max + sqrt(Epsilon);
	}
	
	// The Pt structure represents a point in 2D space.
	struct Pt {
	
		// dot returns the dot product of the two points.
		static double dot(const Pt &a, const Pt &b) { return a.dot(b); }
	
		// sub returns the value of the component-wise subtraction
		// of b from a.
		static Pt sub(const Pt &a, const Pt &b) { return a.minus(b); }
	
		// distance returns the Euclidean distance of two points.
		static double distance(const Pt &a, const Pt &b) {
			Pt diff = b.minus(a);
			return sqrt(dot(diff, diff));
		}
	
		// inf returns a point with infinite values for both x and y.
		static Pt inf(void) { return Pt(Infinity, Infinity); }
	
		// neginf returns a point with negative infinity values for
		// both x and y.
		static Pt neginf(void) { return Pt(-Infinity, -Infinity); }
	
		// angle returns the angle to the point off of the positive x axis.
		// The value is between 0 and 2π.
		static double angle(const Pt &a) {	return a.angle(); }
	
		// cwangle returns the clock-wise angle between the lines from
		// u to v and v to w.
		static double cwangle(const Pt &u, const Pt &v, const Pt &w) {
			Pt a = v.minus(u), b = w.minus(v);
			return -atan2(a.x*b.y - a.y*b.x, a.x*b.x+a.y*b.y);
		}
	
		Pt(void) { }
	
		Pt(const Pt &o) : x(o.x), y(o.y) { }
	
		Pt(double _x, double _y) : x(_x), y(_y) { }
	
		bool isinf(void) { return std::isinf(x) || std::isinf(y); }
	
		bool operator==(const Pt &p) const {
			return doubleeq(x, p.x) && doubleeq(y, p.y);
		}
	
		bool operator!=(const Pt &p) const { return !(*this == p); }
	
		// angle returns the angle to the point off of the positive x axis.
		// The value is between 0 and 2π.
		double angle(void) const {
			double angle = atan2(y, x);
			if (angle < 0)
				return 2 * M_PI + angle;
			return angle;
		}
	
		// dot returns the dot product of this point with point b.
		double dot(const Pt &b) const {
			return x * b.x + y * b.y;
		}
	
		// minus returns the point resulting from component-wise
		// subtraction of b from the current point.
		Pt minus(const Pt &b) const {
			return Pt(x - b.x, y - b.y);
		}
	
		// scale scales the point by the given x and y
		// factors.
		void scale(double sx, double sy) {
			x *= sx;
			y *= sy;
		}
	
		// translate translates the point by the given deltas.
		void translate(double dx, double dy) {
			x += dx;
			y += dy;
		}
	
		// rotates the point by t radians around the origin.
		void rotate(double t) {
			double ct = cos(t), st = sin(t);
			double x0 = x;
			x = x0 * ct - y * st;
			y = x0 * st + y * ct;
		}
	
		double x, y;
	};
	
	struct Bbox {
		Bbox(void) : min(Pt::inf()), max(Pt::neginf()) { }
	
		Bbox(double x0, double y0, double x1, double y1) :
				min(x0, y0), max(x1, y1) {
			normalize();
		}
	
		Bbox(const Pt &p0, const Pt &p1) : min(p0), max(p1) {
			normalize();
		}
	
		// This constructor gets the bounding box for the
		// given set of points
		Bbox(std::vector<Pt> &pts) {
			min = Pt::inf();
			max = Pt::neginf();
	
			for (unsigned int i = 0; i < pts.size(); i++) {
				Pt &p = pts[i];
				if (p.x > max.x)
					max.x = p.x;
				if (p.x < min.x)
					min.x = p.x;
				if (p.y > max.y)
					max.y = p.y;
				if (p.y < min.y)
					min.y = p.y;
			}
		}
	
		bool operator==(const Bbox &o) const {
			return min == o.min && max == o.max;
		}
	
		// contains returns true if the rectangle contains
		// the given point.
		bool contains(const Pt &p) const {
			return between(min.x, max.x, p.x) && between(min.y, max.y, p.y);
		}
	
		// hits returns true if the rectangles intersect
		bool hits(const Bbox &b) const {
			return !(min.x > b.max.x || b.min.x > max.x
				|| min.y > b.max.y || b.min.y > max.y);
		}
	
		// translate translates the rectangle by the given
		// delta values.
		void translate(double dx, double dy) {
			min.translate(dx, dy);
			max.translate(dx, dy);
		}
	
		// center returns the center point of the rectangle.
		Pt center(void) const {
			double dx = max.x - min.x;
			double dy = max.y - min.y;
			return Pt(min.x + dx/2, min.y + dy/2);
		}
	
		Pt min, max;
	
	protected:
	
		void normalize(void) {
			if (min.x > max.x) {
				double t = min.x;
				min.x = max.x;
				max.x = t;
			}
			if (min.y > max.y) {
				double t = min.y;
				min.y = max.y;
				max.y = t;
			}
		}
	};
	
	// The Line struct represents an infinite line in Euclidean space.
	struct Line {
		Line(void) { }
	
		// Line(Pt,Pt) creates a line defined by the two points.
		Line(const Pt &p0, const Pt &p1) {
			double dx = p1.x - p0.x;
			double dy = p1.y - p0.y;
			if (fabs(dx) < Epsilon) {
				m = Infinity;
				b = p1.x;
	 		} else {
				m = dy / dx;
				b = p0.y - m * p0.x;
			}
		}
	
		// isect returns the point of intersection between two lines.
		// If the lines are parallel then Pt::inf is returned.
		Pt isect(const Line &o) const {
			if (doubleeq(m, o.m) || (std::isinf(m) && std::isinf(o.m)))
				return Pt::inf();
			else if (std::isinf(o.m))
				return Pt(o.b, m *o.b + b);
			else if (std::isinf(m))
				return Pt(b, o.m * b + o.b);

			double x = (o.b - b) / (m - o.m);
			return Pt(x, m * x + b);
		}
	
		// isabove returns true if the point is above the given line.
		bool isabove(const Pt &p) const { return (m * p.x + b) < p.y; }
	
		// In case of a vertical line: m == ∞ and b = x
		double m, b;
	};
	
	struct LineSg : public Line {
		LineSg(void) { }
	
		LineSg(const Pt &_p0, const Pt &_p1) :
			Line(_p0, _p1), p0(_p0), p1(_p1), bbox(p0.x, p0.y, p1.x, p1.y)
			{ }
	
		// length returns the length of the line segment.
		double length(void) const {
			return Pt::distance(p0, p1);
		}
	
		// midpt returns the point directly in the middle of the
		// line segment.
		Pt midpt(void) const {
			return Pt((p0.x + p1.x) / 2, (p0.y + p1.y) / 2);
		}
	
		// along returns the point that is the given distance along
		// the line segment from the starting point.  The distance
		// can be greater than the length of the segment.
		Pt along(double dist) const {
			double theta = Pt::angle(p1.minus(p0));
			return Pt(p0.x + dist * cos(theta), p0.y + dist * sin(theta));
		}

		// within returns true if a point (already known to be on
		// the Line) is also within the line segment.
		bool within(const Pt &p) const {
			if (bbox.max.x - bbox.min.x > bbox.max.y - bbox.min.y)
				return between(bbox.min.x, bbox.max.x, p.x);
			return between(bbox.min.y, bbox.max.y, p.y);
		}
	
		// isect returns the point at which the two line segments
		// intersect.  If they do not intersect then Pt::inf() is
		// returned.  If the Line (infinite line) corresponding to
		// the two lines is the same then the return value is
		// also Pt::inf().
		Pt isect(const LineSg &o) const {
			Pt p;
			if (isvertical() && o.isvertical())
				return Pt::inf();
			else if (o.isvertical())
				p = Pt(o.p0.x, m * o.p0.x + b);
			else if (isvertical())
				p = Pt(p0.x, o.m * p0.x + o.b);
			else
				p = Line::isect(o);

			if (between(bbox.min.x, bbox.max.x, p.x) &&
				between(o.bbox.min.x, o.bbox.max.x, p.x))
				return p;
			return Pt::inf();
		}
	
		// hits returns true if the two line segments intersect.
		bool hits(const LineSg &l) const {
			return bbox.hits(l.bbox) && !isect(l).isinf();
		}
	
		// isvertical returns true if the line is a vertical line.
		bool isvertical(void) const { return doubleeq(p0.x, p1.x); }
	
		Pt p0, p1;
		Bbox bbox;
	};
	
	struct QuadEq {
		QuadEq(void) { }
	
		QuadEq(double _a, double _b, double _c) : a(_a), b(_b), c(_c) { }
	
		double discriminant(void) const { return b*b - 4*a*c; }
	
		unsigned int solutions(double s[]) const {
			double d = discriminant();
			 if (d < 0) {
				return 0;
			} else if (d == 0) {
				s[0] = -b / (2*a);
				return 1;
			}
			double sqrtd = sqrt(d);
			s[0] = (-b + sqrtd) / (2*a);
			s[1] = (-b - sqrtd) / (2*a);
			return 2;
		}
	
		double a, b, c;
	};
	
	struct Arc {
	
		// Arc constructs a new arc with the given center
		// radius, initial angle and final angle.  Angles are
		// given in radians.
		Arc(const Pt &_c, double _r, double _t0, double _t1) :
			c(_c), r(_r), t0(_t0), t1(_t1),
			bbox(c.x - r, c.y - r, c.x + r, c.y + r)
			{ }
	
		// rotate rotates the arc by the given number of
		// degrees about the starting point.
		void rotate(double t) {
			Pt p = start();
			c.translate(-p.x, -p.y);
			c.rotate(t);
			c.translate(p.x, p.y);
			bbox = Bbox(c.x - r, c.y - r, c.x + r, c.y + r);
			t0 += t;
			t1 += t;
		}
	
		// translate translates the arc by the given amount in
		// both the x and y directions.
		void translate(double dx, double dy) {
			c.translate(dx, dy);
			bbox = Bbox(c.x - r, c.y - r, c.x + r, c.y + r);
		}
	
		// isects returns the number of isects and returns
		// (via the second argument) the points that intersected.
		// The maximum number of intersections is 2 so the Pt
		// array must be at least 2 elements.
		unsigned int isects(const LineSg &l, Pt is[]) const {
			if (!bbox.hits(l.bbox))
				return 0;
		
			Q q(*this, l);
			double u[2];
			unsigned int n = q.solutions(u);
			assert (n <= 2);
	 
			unsigned int i = 0;
			for (unsigned int j = 0; j < n; j++) {
				is[i].x = l.p0.x + u[j] * q.dx;
				is[i].y = l.p0.y + u[j] * q.dy;
				double t = Pt::angle(Pt(is[i].x - c.x, is[i].y - c.y));
				if (l.within(is[i]) && between(t0, t1, t))
					i++;
			}
	
			return i;
		}
	
		// hits returns true if the linesegment hits the arc
		bool hits(const LineSg &l) const {
			Pt is[2];
			return isects(l, is) > 0;
		}
	
		// start returns the starting point of the arc.
		Pt start(void) const {
			return Pt(c.x + cos(t0) * r, c.y + sin(t0) * r);
		}
	
		// end returns the ending point of the arc.
		Pt end(void) const {
			return Pt(c.x + cos(t1) * r, c.y + sin(t1) * r);
		}
	
		Pt c;
		double r, t0, t1;
		Bbox bbox;
	
	private:
		struct Q : public QuadEq {
			Q(const Arc &arc, const LineSg &l) {
				double cx = arc.c.x, cy = arc.c.y;
				double rr = arc.r*arc.r;
				double x1 = l.p0.x, y1 = l.p0.y;
				dx = l.p1.x - x1, dy = l.p1.y - y1;
				a = dx*dx + dy*dy;
				b = 2 * (dx*(x1 - cx) + dy*(y1 - cy));
				c = cx*cx + cy*cy + x1*x1 + y1*y1 - 2*(cx * x1 + cy * y1) - rr;
			}
	
			double dx, dy;
		};
	};
	
	struct Poly {
	
		// random generates a random polygon for which no side
		// crosses the line between the vertices with the minimum
		// and maximum x value.
		static Poly random(unsigned int n, double xc, double yc, double r);
	
		// giftwrap returns the polygon for the outter-hull of the
		// given set of points.
		static Poly giftwrap(const std::vector<Pt>&);

		// triangle returns a triangle polygon centered at the given
		// point with the specified height, width and rotated at the
		// given angle.
		static Poly triangle(const Pt&, double h, double w, double r);
	
		Poly(const std::vector<Pt>&);
	
		Poly(unsigned int, ...);
	
		Poly(FILE*);
	
		// output writes the polygon to the given file.
		void output(FILE*) const;
	
		// contains returns true if the polygon contains
		// the given point.
		bool contains(const Pt&) const;
	
		// hits returns true if the line segment intersects
		// one of the sides of the polygon.
		bool hits(const LineSg &l) const {
			if (!bbox.hits(l.bbox))
				return false;
			for (unsigned int i = 0; i < sides.size(); i++) {
				const LineSg &side = sides[i];
				if (l.hits(side))
					return true;
			}
			return false;
		}
	
		// hits returns true if the arc intersects one of the
		// sides of the polygon.
		bool hits(const Arc &a) const {
			if (!bbox.hits(a.bbox))
				return false;
			for (unsigned int i = 0; i < sides.size(); i++) {
				const LineSg &side = sides[i];
				if (a.hits(side))
					return true;
			}
			return false;
		}
	
		// minisect returns the first intersection point between
		// the line segment and the polygon.
		Pt minisect(const LineSg&) const;
	
		// isects returns a list of all intersections between
		// the line segment and the polygon.
		std::vector<Pt> isects(const LineSg&) const;
	
		// scale scales the polygon by the given factors
		// in both the x and y directions.
		void scale(double sx, double sy) {
			for (unsigned long i = 0; i < verts.size(); i++)
				verts[i].scale(sx, sy);
			bbox = Bbox(verts);
			initsides();
		}
	
		// translate translates the polygon by the given
		// values in both the x and y directions.
		void translate(double dx, double dy) {
			bbox.translate(dx, dy);
			for (unsigned int i = 0; i < verts.size(); i++)
				verts[i].translate(dx, dy);
			initsides();
		}
	
		bool isreflex(unsigned int i) const { return interangle(i) < M_PI; }
	 
		std::vector<Pt> verts;
		std::vector<LineSg> sides;
		Bbox bbox;
	
	private:
		double interangle(unsigned int) const;
		void removecolinear(void);
		void initsides(void);
	};
};

#endif	// _GEOM2D_HPP_