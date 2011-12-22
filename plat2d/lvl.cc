#include "lvl.hpp"
#include "tile.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"
#include <cstring>
#include <cmath>
#include <cerrno>

Lvl::Lvl(unsigned int _w, unsigned int _h) : w(_w), h(_h) {
	blks = new Blk[w * h];
	memset(blks, 0, w * h * sizeof(*blks));
}

Lvl::Lvl(FILE *in) {
	read(in);
}

Lvl::~Lvl(void) {
	delete[] blks;
}

void Lvl::read(FILE *f)
{
	unsigned int d;
	int ret = fscanf(f, " %u %u %u",&d, &w, &h);
	if (ret == EOF && ferror(f))
		fatal("Failed to read the level");
	if (ret != 3)
		fatal("Malformed level header");
	if (d != 1)
		fatal("Only levels with a depth of 1 are supported");

	blks = new Blk[w * h * d];
	memset(blks, 0, w * h * d * sizeof(*blks));

	if (fgetc(f) != '\n')
		fatal("Expected a new line at z=0");

	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) 
			blk(x, y)->tile = Tile::read(f);
		if (fgetc(f) != '\n')
			fatal("Expected a new line at y=%u\n", y);
	}
}

struct Hitzone {
	Hitzone(Bbox a, const Point &v) {
		Bbox b(a);
		b.translate(v.x, v.y);

		x0 = a.min.x / (double) Tile::Width;
		y0 = a.min.y / (double) Tile::Height;
		x1 = ceil(a.max.x) / (double) Tile::Width;
		y1 = ceil(a.max.y) / (double) Tile::Height;
		if (y0 > 0)
			y0--;
		if (x0 > 0)
			x0--;
	}

	unsigned int x0, y0, x1, y1;
};

Isect Lvl::isection(const Bbox &r, const Point &v) const {
	Hitzone test(r, v);
	Isect isect;
	Bbox mv(r);

	mv.translate(0, v.y);
	for (unsigned int x = test.x0; x <= test.x1; x++) {
	for (unsigned int y = test.y0; y <= test.y1; y++) {
		unsigned int t = blks[ind(x, y)].tile;
		Isect is(tiles[t].isection(x, y, mv));
		if (is.is && is.dy > isect.dy) {
			isect.is = true;
			isect.dy = is.dy;
		}
	}
	}

	if (v.x == 0.0)
		return isect;

	mv = r;
	mv.translate(v.x, v.y + (v.y < 0 ? isect.dy : -isect.dy));

	for (unsigned int x = test.x0; x <= test.x1; x++) {
	for (unsigned int y = test.y0; y <= test.y1; y++) {
		unsigned int t = blks[ind(x, y)].tile;
		Isect is(tiles[t].isection(x, y, mv));
		if (is.is && is.dx > isect.dx) {
			isect.is = true;
			isect.dx = is.dx;
		}
	}
	}

	return isect;
}

void Lvl::draw(Image &img) const {
	unsigned int ht =  h * Tile::Height;

	for (unsigned int y = 0; y < h; y++) {
	for (unsigned int x = 0; x < w; x++) {
		unsigned int t = blks[ind(x, y)].tile;

		double xpos = x * Tile::Width;
		double ypos = ht - (y + 1) * Tile::Height;

		if (tiles[t].flags & Tile::Water)
			Tile::draw(img, xpos, ypos, Image::blue);
		else if (tiles[t].flags & Tile::Collide)
			Tile::draw(img, xpos, ypos, Image::black);

		if (tiles[t].flags & (Tile::Up | Tile::Down)) {
			xpos += 0.5 * Tile::Width;
			ypos += 0.5 * Tile::Height;
			double rot = 90;
			if (tiles[t].flags & Tile::Down)
				rot = 270;
			img.add(new Image::Triangle(xpos, ypos, Tile::Width,
				45.0, rot, Image::black));
		}
	}
	}
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

	Polygon poly(double sx, double sy) {
		std::vector<Point> pts;

		Pose cur(minx, miny, Up);
		pts.push_back(Point(minx * sx, miny * sy));

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
				pts.push_back(Point(cur.x * sx, (cur.y+1) * sy));
				break;

			case Down:
				pts.push_back(Point((cur.x+1) * sx, cur.y * sy));
				break;

			case Right:
				pts.push_back(Point((cur.x+1) * sx, (cur.y+1) * sy));
				break;

			case Left:
				pts.push_back(Point(cur.x * sx, cur.y * sy));
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

static std::vector<Component> components(const Lvl &lvl) {
	unsigned int w = lvl.width(), h = lvl.height();
	unsigned int sz = w * h;
	Djset *forest = new Djset[sz];

	for (unsigned int x = 0; x < w; x++) {
	for (unsigned int y = 0; y < h; y++) {
		if (!lvl.blocked(x, y))
			continue;

		if (x < w - 1 && lvl.blocked(x+1, y))
			forest[x*h + y].join(forest[(x + 1)*h + y]);

		if (y < h - 1 && lvl.blocked(x, y+1) && y != h/2)
			forest[x*h + y].join(forest[x*h + y + 1]);

		if (y < h - 1 && x > 0 && lvl.blocked(x-1, y+1) && y != h/2)
			forest[x*h + y].join(forest[(x - 1)*h + y + 1]);

		if (y < h - 1 && x < w - 1 && lvl.blocked(x+1, y+1) && y != h/2)
			forest[x*h + y].join(forest[(x + 1)*h + y + 1]);
	}
	}

	std::vector<Component> cs;

	for (unsigned int x = 0; x < w; x++) {
	for (unsigned int y = 0; y < h; y++) {
		if (!lvl.blocked(x, y))
			continue;
		Djset *s = forest[x * h + y].find();
		if (!s->aux) {
			cs.push_back(Component(w, h));
			s->aux = (void*)cs.size();
		}
		cs[(uintptr_t) s->aux - 1].add(x, h-y-1);
	}
	}

	return cs;
}

std::vector<Polygon> Lvl::polys(void) const {
	std::vector<Component> comps = components(*this);
	std::vector<Polygon> polys;
	// 2.0 + 5.0 / 4.0 is from Player::runspeed()
	double w = 1; //Tile::Width / (2.0 + 5.0 / 4.0);
	double h = 1; // Tile::Height / Body::Maxdy;
	for (unsigned int i = 0; i < comps.size(); i++)
		polys.push_back(comps[i].poly(w, h));
	return polys;
}