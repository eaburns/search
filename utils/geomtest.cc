#include "geom.hpp"
#include "image.hpp"
#include "../visnav/polymap.hpp"
#include <cstdio>

int main(void) {
	PolyMap map(stdin);

	Point min = map.min();
	min.x = ceil(min.x);
	min.y = ceil(min.y);

	Point max = map.max();
	max.x = ceil(max.x);
	max.y = ceil(max.y);

	Rectangle bbox(min.x, min.y, max.x, max.y);
	Point c = max;
	double t0 = 0, t1 = 2 * M_PI;
	double r = 50;

	for (unsigned int i = 0; i < 10; i++) {
		Image img(max.x, max.y);
		Arc a(c, r, t0, t1);
		a.draw(img, Color(1, 0, 0), 1);
		map.draw(img);

		for (unsigned int pid = 0; pid < map.polys.size(); pid++) {
			const Polygon &p = map.polys[pid];
			for (unsigned int sid = 0; sid < p.sides.size(); sid++) {
				const LineSeg &s = p.sides[sid];
				Point hits[2];
				unsigned int n = a.isections(s, hits);
				for (unsigned int j = 0; j < n; j++)
					img.add(new Image::Circle(hits[j].x, hits[j].y, 2, Color(0.5, 0.5, 0.5)));
			}
		}
		for (unsigned int sid = 0; sid < map.bound->sides.size(); sid++) {
			const LineSeg &s = map.bound->sides[sid];
			Point hits[2];
			unsigned int n = a.isections(s, hits);
			for (unsigned int j = 0; j < n; j++)
				img.add(new Image::Circle(hits[j].x, hits[j].y, 2, Color(0.5, 0.5, 0.5)));
		}

		char name[256];
		snprintf(name, sizeof(name), "geomtest%03u.eps", i);
		img.save(name);

		c.x -= r;
		c.y -= r;
		double diff = t1 - t0;
		t0 += M_PI / 5;
		t1 = t0 + (diff * 0.75);
	}

	return 0;
}