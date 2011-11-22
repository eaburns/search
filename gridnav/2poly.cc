#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include "../visnav/visgraph.hpp"
#include <vector>

typedef std::vector<unsigned int> Comp;

static std::vector<Comp*> comps(const GridMap&);
static Polygon poly(const GridMap&, const Comp*);
static void adduniq(std::vector<Point>&, const Point&);
Polygon giftwrap(double, const std::vector<Point>&);
static unsigned int minx(const std::vector<Point>&);

int main(int argc, char *argv[]) {
	GridMap map(stdin);

	std::vector<Comp*> cs = comps(map);

	std::vector<Polygon> ps;
	for (unsigned int i = 0; i < cs.size(); i++)
		ps.push_back(poly(map, cs[i]));

	while (cs.size() > 0) {
		delete cs.back();
		cs.pop_back();
	}

	VisGraph vg(ps);
	vg.output(stdout);

	return 0;
}

static std::vector<Comp*> comps(const GridMap &map) {
	Djset *forest = new Djset[map.size()];

	for (unsigned int l = 0; l < map.size(); l++) {
		if (map.passable(l))
			continue;

		unsigned int x = map.x(l), y = map.y(l);
		if (x < map.width() - 1 && !map.passable(map.right(l)))
			forest[l].join(forest[map.right(l)]);

		// split map in half to ensure outter boundry is a polygon
		if (y < map.height() - 1 && !map.passable(map.down(l)) &&
 			(y != map.height() / 2 || forest[l].find() != forest[0].find()))
			forest[l].join(forest[map.down(l)]);
	}

	std::vector<Comp*> cs;
	for (unsigned int l = 0; l < map.size(); l++) {
		if (map.passable(l))
			continue;

		Djset *s = forest[l].find();
		if (!s->aux) {
			cs.push_back(new Comp());
			s->aux = cs[cs.size() - 1];
		}
		((Comp*) s->aux)->push_back(l);
	}

	delete[] forest;

	return cs;
}

#include <cassert>

static Polygon poly(const GridMap &map, const Comp *c) {
	std::vector<Point> pts;
	double w = 1.0 / map.width();
	double h = 1.0 / map.height();

	for (unsigned int i = 0; i < c->size(); i++) {
		assert (!map.passable(c->at(i)));
		unsigned int x = map.x(c->at(i));
		unsigned int y = map.y(c->at(i));
		adduniq(pts, Point(x * w, y * h));
		adduniq(pts, Point((x + 1) * w, y * h));
		adduniq(pts, Point((x + 1) * w, (y + 1) * h));
		adduniq(pts, Point(x * w, (y + 1) * h));
	}

	return giftwrap(w > h ? w : h, pts);
}

static void adduniq(std::vector<Point> &pts, const Point &p) {
	for (unsigned int i = 0; i < pts.size(); i++) {
		if (pts[i].x == p.x && pts[i].y == p.y)
			return;
	}
	pts.push_back(p);
}

Polygon giftwrap(double d, const std::vector<Point> &pts) {
	std::vector<Point> hull;
	unsigned int min = minx(pts);

	unsigned int cur = min, prev = pts.size();
	Point prevpt(pts[cur].x, pts[cur].y - 0.1);
	double prevangle = FP_NAN;

	do {
		double nextangle = std::numeric_limits<double>::infinity();
		unsigned int next = cur;
		for (unsigned int i = 0; i < pts.size(); i++) {
			double dist = Point::distance(pts[cur], pts[i]);
			if (i == prev || i == cur || dist > d * 1.01)
				continue;
			double t = Point::cwangle(prevpt, pts[cur], pts[i]);
			if (t < nextangle) {
				nextangle = t;
				next = i;
			}
		}

		for (unsigned int i = 0; i < hull.size(); i++)
			assert(hull[i] != pts[next]);

		hull.push_back(pts[next]);

		prev = cur;
		prevpt = pts[cur];
		prevangle = nextangle;
		cur = next;
	} while (cur != min);

	return Polygon(hull);
}

static unsigned int minx(const std::vector<Point> &pts) {
	unsigned int min = pts.size();
	for (unsigned int i = 0; i < pts.size(); i++) {
		if (min >= pts.size() || pts[i].x < pts[min].x)
			min = i;
	}
	return min;
}