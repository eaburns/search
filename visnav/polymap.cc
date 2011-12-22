#include "polymap.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"
#include <cerrno>

PolyMap::PolyMap(FILE *in) {
	input(in);
}

void PolyMap::input(FILE *in) {
	unsigned int npoly;
	int res = fscanf(in, " %u polygons\n", &npoly);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the the visibility map");
	if (res != 1)
		fatal("Malformed visibility map");

	for (unsigned int i = 0; i < npoly; i++)
		polys.push_back(Polygon(in));
}

void PolyMap::output(FILE *out) const {
	fprintf(out, "%u polygons\n", (unsigned int) polys.size());
	for (unsigned int i = 0; i < polys.size(); i++) {
		polys[i].output(out);
		putc('\n', out);
	}
}

void PolyMap::draw(Image &img, double lwidth) const {
	for (unsigned int i = 0; i < polys.size(); i++) {
		const Color &c = somecolors[i % Nsomecolors];
		polys[i].draw(img, c, lwidth);
	}
}

void PolyMap::scale(double sx, double sy) {
	for (unsigned int i = 0; i < polys.size(); i++)
		polys[i].scale(sx, sy);		
}

void PolyMap::translate(double dx, double dy) {
	for (unsigned int i = 0; i < polys.size(); i++)
		polys[i].translate(dx, dy);		
}

Point PolyMap::min(void) const {
	Point min = Point::inf();
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (polys[i].bbox.min.x < min.x)
			min.x = polys[i].bbox.min.x;
		if (polys[i].bbox.min.y < min.y)
			min.y = polys[i].bbox.min.y;
	}
	return min;
}

Point PolyMap::max(void) const {
	Point max = Point::neginf();
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (polys[i].bbox.max.x > max.x)
			max.x = polys[i].bbox.max.x;
		if (polys[i].bbox.min.y > max.y)
			max.y = polys[i].bbox.max.y;
	}
	return max;
}

bool PolyMap::isvisible(const Point &a, const Point &b) const {
	const LineSeg line(a, b);
	for (unsigned int i = 0; i < polys.size(); i++) {
		if (polys[i].hits(line))
			return false;
	}
	return true;
}

struct Component {

	Component(unsigned int _w, unsigned int _h) :
		w(_w), h(_h), n(0), blks(w * h) { }

	void add(unsigned int x, unsigned int y) {
		if (n == 0 || x < minx) {
			minx = x;
			miny = y;
		}
		n++;
		if (x == minx && y < miny)
			miny = y;
		blks[x * h + y] = true;
	}

	Polygon poly(void) {
		std::vector<Point> pts;

		Pose cur(minx, miny, Up);
		pts.push_back(Point(minx, miny));

		for ( ; ; ) {
			Pose next = clockwise(cur);
			if (cur.dir == next.dir) {
				cur = next;
				continue;
			}
			if (next.x == minx && next.y == miny && next.dir == Up)
				break;
			switch (cur.dir) {
			case Up:
				pts.push_back(Point(cur.x, cur.y+1));
				break;

			case Down:
				pts.push_back(Point(cur.x+1, cur.y));
				break;

			case Right:
				pts.push_back(Point(cur.x+1, cur.y+1));
				break;

			case Left:
				pts.push_back(Point(cur.x, cur.y));
				break;
			}
			cur = next;
		}

		return Polygon(pts);
	}

private:

	enum Dir { Up, Down, Left, Right };

	struct Pose {
		Pose(unsigned int _x, unsigned int _y, Dir _dir) :
			x(_x), y(_y), dir(_dir) { }
		unsigned int x, y;
		Dir dir;
	};

	Pose clockwise(const Pose &cur) {
		switch (cur.dir) {
		case Up:
			if (blkd(cur.x-1, cur.y+1))
				return Pose(cur.x-1, cur.y+1, Left);
			if (blkd(cur.x, cur.y+1))
				return Pose(cur.x, cur.y+1, Up);
			return Pose(cur.x, cur.y, Right);

		case Down:
			if (blkd(cur.x+1, cur.y-1))
				return Pose(cur.x+1, cur.y-1, Right);
			if (blkd(cur.x, cur.y-1))
				return Pose(cur.x, cur.y-1, Down);
			return Pose(cur.x, cur.y, Left);

		case Left:
			if (blkd(cur.x-1, cur.y-1))
				return Pose(cur.x-1, cur.y-1, Down);
			if (blkd(cur.x-1, cur.y))
				return Pose(cur.x-1, cur.y, Left);
			return Pose(cur.x, cur.y, Up);

		default:
			break;
		}
		if (blkd(cur.x+1, cur.y+1))
			return Pose(cur.x+1, cur.y+1, Up);
		if (blkd(cur.x+1, cur.y))
			return Pose(cur.x+1, cur.y, Right);
		return Pose(cur.x, cur.y, Down);
	}

	bool blkd(unsigned int x, unsigned int y) const {
		if (x >= w || y >= w)
			return false;
		return blks[x * h + y];
	}

	unsigned int w, h;
	unsigned int minx, miny;
	unsigned int n;
	std::vector<bool> blks;
};