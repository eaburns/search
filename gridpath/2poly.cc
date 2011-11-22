#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include "../visnav/visgraph.hpp"
#include <vector>

typedef std::vector<unsigned int> Comp;

static std::vector<Comp*> comps(const GridMap&);
static Polygon poly(const GridMap&, const Comp*);
static void adduniq(std::vector<Point>&, const Point&);

int main(int argc, char *argv[]) {
	GridMap map(stdin);

	std::vector<Comp*> cs = comps(map);

	std::vector<Polygon> ps;
	for (unsigned int i = 0; i < cs.size(); i++) {
		fprintf(stderr, "wrapping component: %6u/%u\n", i, (unsigned int) cs.size());
		fflush(stderr);
		ps.push_back(poly(map, cs[i]));
	}
	fprintf(stderr, "\n");

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
		fprintf(stderr, "\rfinding components: %6u/%u", l, (unsigned int) map.size());
		fflush(stderr);
		if (map.passable(l))
			continue;

		unsigned int x = map.x(l), y = map.y(l);
		if (x < map.width() - 1 && !map.passable(map.right(l)))
			forest[l].join(forest[map.right(l)]);

		// split map in half to ensure outter boundry is a polygon
		if (y < map.height() - 1 && map.y(l) != map.height() / 2 &&
				!map.passable(map.down(l)))
			forest[l].join(forest[map.down(l)]);
	}
	fprintf(stderr, "\n");

	std::vector<Comp*> cs;
	for (unsigned int l = 0; l < map.size(); l++) {
		if (map.passable(l))
			continue;

		fprintf(stderr, "\rpopulating components: %6u/%u (%lu)", l, (unsigned int) map.size(), cs.size());
		fflush(stderr);
		Djset *s = forest[l].find();
		if (!s->aux) {
			cs.push_back(new Comp());
			s->aux = cs[cs.size() - 1];
		}
		((Comp*) s->aux)->push_back(l);
	}
	fprintf(stderr, "\n");

	delete[] forest;

	return cs;
}

static Polygon poly(const GridMap &map, const Comp *c) {
	std::vector<Point> pts;
	double w = 1.0 / map.width();
	double h = 1.0 / map.height();

	for (unsigned int i = 0; i < c->size(); i++) {
		unsigned int x = map.x(c->at(i));
		unsigned int y = map.y(c->at(i));
		fprintf(stderr, "%u,%u\n", x, y);
		adduniq(pts, Point(x * w, y * h));
		adduniq(pts, Point((x + 1) * w, y * h));
		adduniq(pts, Point((x + 1) * w, (y + 1) * h));
		adduniq(pts, Point(x * w, (y + 1) * h));
	}
	fprintf(stderr, "\n\n");

	return Polygon::giftwrap(pts);
}

static void adduniq(std::vector<Point> &pts, const Point &p) {
	for (unsigned int i = 0; i < pts.size(); i++) {
		if (pts[i].x == p.x && pts[i].y == p.y)
			return;
	}
	pts.push_back(p);
}