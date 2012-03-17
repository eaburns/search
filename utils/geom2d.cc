#include "geom2d.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdarg>
#include <cerrno>

namespace geom2d {
	
	static unsigned int minx(const std::vector<Pt>&);
	static void xsortedpts(std::vector<Pt>&, double, double, double);
	static bool cmpx(const Pt&, const Pt&);
	
	// vertices are given clock-wise around the polygon.
	Poly::Poly(const std::vector<Pt> &vs) : verts(vs), bbox(verts) {
		if (verts.size() < 3)
			fatal("A polygon needs at least 3 points\n");
		removecolinear();
		initsides();
	}
	
	Poly::Poly(unsigned int n, ...) {
		va_list ap;
		va_start(ap, n);
		for (unsigned int i = 0; i < n; i++) {
			double x = va_arg(ap, double);
			double y = va_arg(ap, double);
			verts.push_back(Pt(x, y));
		}
		va_end(ap);
		bbox = Bbox(verts);
		removecolinear();
		initsides();
	}
	
	Poly::Poly(FILE *in) {
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
			verts.push_back(Pt(x, y));
		}
		bbox = Bbox(verts);
		removecolinear();
		initsides();
	}
	
	Poly Poly::random(unsigned int n, double xc, double yc, double r) {
		if (n < 3)
			fatal("A polygon needs at least 3 points\n");
	
		std::vector<Pt> sorted(n);
		xsortedpts(sorted, xc, yc, r);
	
		Line l(sorted[0], sorted[sorted.size()-1]);
		std::vector<Pt> verts, rev;
	
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
	
		return Poly(verts);
	}
	
	Poly Poly::giftwrap(const std::vector<Pt> &pts) {
		std::vector<Pt> hull;
		unsigned int min = minx(pts);
	
		unsigned int cur = min, prev = pts.size();
		Pt prevpt(pts[cur].x, pts[cur].y - 0.1);
	
		do {
			unsigned int next = 0;
			double ang = std::numeric_limits<double>::infinity();
			for (unsigned int i = 0; i < pts.size(); i++) {
				if (i == prev || i == cur)
					continue;
				double t = Pt::cwangle(prevpt, pts[cur], pts[i]);
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
	
		return Poly(hull);
	}

	Poly Poly::triangle(const Pt &c, double h, double w, double r) {
		double side = h / cos(w / 2);
		double xtop = h / 2;
		double xbot = -h / 2;
		double y1 = side * sin(w / 2);
		double y2 = -side * sin(w / 2);
		double cr = cos(r), sr = sin(r);
	
		std::vector<Pt> pts;
		pts.push_back(Pt(xtop * cr + c.x, xtop * sr + c.y));
		pts.push_back(Pt(xbot * cr - y1 * sr + c.x, xbot * sr + y1 * cr + c.y));
		pts.push_back(Pt(xbot * cr - y2 * sr + c.x, xbot * sr + y2 * cr + c.y));

		return Poly(pts);
	}
	
	void Poly::output(FILE *out) const {
		fprintf(out, "%lu", (unsigned long) verts.size());
		for (unsigned int i = 0; i < verts.size(); i++)
			fprintf(out, " %g %g", verts[i].x, verts[i].y);
	}
	
	bool Poly::contains(const Pt &pt) const {
		bool even = true, isect = false;
		Line ray(pt, Pt(pt.x + 1, pt.y));
	
		for (unsigned int i = 0; i < sides.size(); i++) {
			const LineSg &side = sides[i];
			if (side.bbox.max.x < pt.x || side.bbox.max.y < pt.y || side.bbox.min.y > pt.y)
				continue;
			Pt hit = ray.isect(side);
	
			if (hit.isinf() || hit.x <= pt.x || !side.within(hit))
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
	
	std::vector<Pt> Poly::isects(const LineSg &l) const {
		std::vector<Pt> is;
	
		for (unsigned int i = 0; i < sides.size(); i++) {
			Pt p = l.isect(sides[i]);
			if (!p.isinf())
				is.push_back(p);
		}
	
		return is;
	}
	
	Pt Poly::minisect(const LineSg &l) const {
		Pt min = Pt::inf();
		double mindist = std::numeric_limits<double>::infinity();
	
		for (unsigned int i = 0; i < sides.size(); i++) {
			Pt p = l.isect(sides[i]);
			double d = Pt::distance(p, l.p0);
			if (!p.isinf() && d < mindist) {
				mindist = d;
				min = p;
			}
		}
	
		return min;
	}
	
	double Poly::interangle(unsigned int i) const {
		const Pt &u = i == 0 ? verts[verts.size() - 1] : verts[i-1];
		const Pt &v = verts[i];
		const Pt &w = i == verts.size() - 1 ? verts[0] : verts[i+1];
		Pt a = v.minus(u), b = w.minus(v);
	
		return M_PI - fmod(atan2(b.x*a.y - a.x*b.y, b.x*a.x + b.y*a.y), 2 * M_PI);
	}
	
	void Poly::removecolinear(void) {
		for (unsigned int i = 0; i < verts.size(); ) {
			if (fabs(interangle(i) - M_PI) < Epsilon)
				verts.erase(verts.begin() + i);
			else
				i++;
		}
	}
	
	void Poly::initsides(void) {
		sides.clear();
		for (unsigned int i = 1; i < verts.size(); i++)
			sides.push_back(LineSg(verts[i-1], verts[i]));
		sides.push_back(LineSg(verts[verts.size() - 1], verts[0]));
	}
	
	static unsigned int minx(const std::vector<Pt> &pts) {
		unsigned int min = pts.size();
		for (unsigned int i = 0; i < pts.size(); i++) {
			if (min >= pts.size() || pts[i].x < pts[min].x)
				min = i;
		}
		return min;
	}
	
	static void xsortedpts(std::vector<Pt> &pts, double xc, double yc, double r) {
		for (unsigned int i = 0; i < pts.size(); i++) {
			double x = (2 * r * randgen.real()) - r;
			double y = (2 * r * randgen.real()) - r;
			pts[i] = Pt(x + xc, y + yc);
		}
	
		std::sort(pts.begin(), pts.end(), cmpx);
	}
	
	static bool cmpx(const Pt &a, const Pt &b) {
		return a.x < b.x;
	}
	
};