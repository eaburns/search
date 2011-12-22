#include "../utils/utils.hpp"
#include "polymap.hpp"
#include <vector>

struct Comp {
	Comp(unsigned int _w, unsigned int _h) :
		w(_w), h(_h), n(0), blkd(w * h, false) { }

	void add(unsigned int x, unsigned int y) {
		if (n == 0 || x < minx) {
			minx = x;
			miny = y;
		}
		n++;
		if (x == minx && y < miny)
			miny = y;
		blkd[x*h + y] = true;
	}

	Polygon poly(void) {
		std::vector<Point> pts;
		Pose cur(minx, miny, Pose::Up);
		pts.push_back(Point(minx, miny));

		for ( ; ; ) {
			Pose next = clockwise(cur);

			if (cur.dir == next.dir) {
				cur = next;
				continue;
			}

			if (next.x == minx && next.y == miny && next.dir == Pose::Up)
				break;
			switch (cur.dir) {
			case Pose::Up:
				pts.push_back(Point(cur.x, cur.y+1));
				break;

			case Pose::Down:
				pts.push_back(Point(cur.x+1, cur.y));
				break;

			case Pose::Right:
				pts.push_back(Point(cur.x+1, cur.y+1));
				break;

			case Pose::Left:
				pts.push_back(Point(cur.x, cur.y));
				break;
			}
			cur = next;
		}

		return Polygon(pts);
	}

	struct Pose {
		enum Dir { Up, Down, Left, Right };

		Pose(unsigned int _x, unsigned int _y, Dir _dir) :
			x(_x), y(_y), dir(_dir) { }

		unsigned int x, y;
		Dir dir;
	};

	Pose clockwise(const Pose &cur) {
		switch (cur.dir) {
		case Pose::Up:
			if (blocked(cur.x-1, cur.y+1))
				return Pose(cur.x-1, cur.y+1, Pose::Left);
			if (blocked(cur.x, cur.y+1))
				return Pose(cur.x, cur.y+1, Pose::Up);
			return Pose(cur.x, cur.y, Pose::Right);

		case Pose::Down:
			if (blocked(cur.x+1, cur.y-1))
				return Pose(cur.x+1, cur.y-1, Pose::Right);
			if (blocked(cur.x, cur.y-1))
				return Pose(cur.x, cur.y-1, Pose::Down);
			return Pose(cur.x, cur.y, Pose::Left);

		case Pose::Left:
			if (blocked(cur.x-1, cur.y-1))
				return Pose(cur.x-1, cur.y-1, Pose::Down);
			if (blocked(cur.x-1, cur.y))
				return Pose(cur.x-1, cur.y, Pose::Left);
			return Pose(cur.x, cur.y, Pose::Up);

		default:
			break;
		}
		if (blocked(cur.x+1, cur.y+1))
			return Pose(cur.x+1, cur.y+1, Pose::Up);
		if (blocked(cur.x+1, cur.y))
			return Pose(cur.x+1, cur.y, Pose::Right);
		return Pose(cur.x, cur.y, Pose::Down);
	}

	bool blocked(unsigned int x, unsigned int y) const {
		return x < w && y < h && blkd[x * h + y];
	}

	unsigned int minx, miny;
	unsigned int w, h;
	unsigned int n;
	std::vector<bool> blkd;
};

PolyMap::PolyMap(bool blkd[], unsigned int w, unsigned int h) {
	Djset *forest = new Djset[w * h];
	for (unsigned int x = 0; x < w; x++) {
	for (unsigned int y = 0; y < h; y++) {
		if (!blkd[x*h + y])
			continue;

		if (x < w - 1 && blkd[(x+1)*h + y])
			forest[x*h + y].join(forest[(x + 1)*h + y]);

		if (y < h - 1 && blkd[x*h + y + 1] && y != h/2)
			forest[x*h + y].join(forest[x*h + y + 1]);

		if (y < h - 1 && x > 0 && blkd[(x - 1)*h + y + 1] && y != h/2)
			forest[x*h + y].join(forest[(x - 1)*h + y + 1]);

		if (y < h - 1 && x < w - 1 && blkd[(x + 1) * h + y + 1] && y != h/2)
			forest[x*h + y].join(forest[(x + 1)*h + y + 1]);
	}
	}

	std::vector<Comp> cs;
	for (unsigned int x = 0; x < w; x++) {
	for (unsigned int y = 0; y < h; y++) {
		if (!blkd[x*h + y])
			continue;
		Djset *s = forest[x * h + y].find();
		if (!s->aux) {
			cs.push_back(Comp(w, h));
			s->aux = (void*)cs.size();
		}
		cs[(uintptr_t) s->aux - 1].add(x, y);
	}
	}

	for (unsigned int i = 0; i < cs.size(); i++)
		polys.push_back(cs[i].poly());
}