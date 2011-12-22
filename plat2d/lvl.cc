#include "lvl.hpp"
#include "tile.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"
#include <cstring>
#include <cmath>

Lvl::Lvl(unsigned int _w, unsigned int _h, unsigned int _d) :
		w(_w), h(_h), d(_d) {
	blks = new Blk[w * h * d];
	memset(blks, 0, w * h * d * sizeof(*blks));
}

Lvl::Lvl(FILE *in) {
	read(in);
}

Lvl::~Lvl(void) {
	delete[] blks;
}

void Lvl::read(FILE *f)
{
	if (fscanf(f, " %d %d %d",&d, &w, &h) != 3)
		fatal("Invalid lvl header: w = %d, h = %d, d = %d, feof = %d, ferror = %d", w, h, d, feof(f), ferror(f));

	blks = new Blk[w * h * d];
	memset(blks, 0, w * h * d * sizeof(*blks));

	if (fgetc(f) != '\n')
		fatal("Expected a new line at z=0");

	for (unsigned int z = 0; z < d; z++) {
		for (unsigned int y = 0; y < h; y++) {
			for (unsigned int x = 0; x < w; x++) {
				int c = Tile::read(f);
				blk(x, y, z)->tile = c;
				if (z == 0 && tiles[c].flags & Tile::Fdoor)
					fatal("Front door on x=%d, y=%d, z=0", x, y);
				if (z == d - 1 && tiles[c].flags & Tile::Bdoor)
					fatal("Back door on x=%d, y=%d, z=max", x, y);
			}
			if (fgetc(f) != '\n')
				fatal("Expected a new line at z=%u, y=%u\n", z, y);
		}
		if (z < d - 1 && fgetc(f) != '\n')
			fatal("Expected a new line at z=%u\n", z);
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

Isect Lvl::isection(unsigned int z, const Bbox &r, const Point &v) const {
	Hitzone test(r, v);
	Isect isect;
	Bbox mv(r);

	mv.translate(0, v.y);
	for (unsigned int x = test.x0; x <= test.x1; x++) {
	for (unsigned int y = test.y0; y <= test.y1; y++) {
		unsigned int t = blks[ind(x, y, z)].tile;
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
		unsigned int t = blks[ind(x, y, z)].tile;
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
	unsigned int ht =  d * h * Tile::Height;

	for (unsigned int z = 0; z < d; z++) {
	for (unsigned int y = 0; y < h; y++) {
	for (unsigned int x = 0; x < w; x++) {
		unsigned int t = blks[ind(x, y, z)].tile;

		double xpos = x * Tile::Width;
		double ypos = ht - (z * h + y + 1) * Tile::Height;

		if (tiles[t].flags & Tile::Water)
			Tile::draw(img, xpos, ypos, Image::blue);
		else if (tiles[t].flags & Tile::Collide)
			Tile::draw(img, xpos, ypos, Image::black);

		if (tiles[t].flags & (Tile::Bdoor | Tile::Fdoor | Tile::Up | Tile::Down)) {
			xpos += 0.5 * Tile::Width;
			ypos += 0.5 * Tile::Height;
			double rot = 0;
			if (tiles[t].flags & Tile::Up)
				rot = 90;
			else if (tiles[t].flags & Tile::Fdoor)
				rot = 180;
			else if (tiles[t].flags & Tile::Down)
				rot = 270;
			img.add(new Image::Triangle(xpos, ypos, Tile::Width,
				45.0, rot, Image::black));
		}
	}
	}
	}

	for (unsigned int z = 0; z < d; z++) {
		img.add(new Image::Rect(
			0,
			ht - (z + 1) * h * Tile::Height,
			w * Tile::Width,
			h * Tile::Height,
			Image::red, 25.0));
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
			const char *dir = "up";
			switch (cur.dir) {
			case Down: dir = "down"; break;
			case Left: dir = "left"; break;
			case Right: dir = "right"; break;
			default: break;
			}
			fprintf(stderr, "cur=%u,%u, %s\n", cur.x, cur.y, dir);
			Pose next = clockwise(cur);
			if (cur.dir == next.dir) {
				cur = next;
				continue;
			}
			if (next.x == minx && next.y == miny && next.dir == Up)
				break;
			switch (cur.dir) {
			case Up:
				fprintf(stderr, "up placing %u,%u\n", cur.x, cur.y+1);
				pts.push_back(Point(cur.x * sx, (cur.y+1) * sy));
				break;

			case Down:
				fprintf(stderr, "down placing %u,%u\n", cur.x+1, cur.y);
				pts.push_back(Point((cur.x+1) * sx, cur.y * sy));
				break;

			case Right:
				fprintf(stderr, "right placing %u,%u\n", cur.x+1, cur.y+1);
				pts.push_back(Point((cur.x+1) * sx, (cur.y+1) * sy));
				break;

			case Left:
				fprintf(stderr, "left placing %u,%u\n", cur.x, cur.y);
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

static std::vector<Component> components(const Lvl &lvl, unsigned int z) {
	unsigned int w = lvl.width(), h = lvl.height();
	unsigned int sz = w * h;
	Djset *forest = new Djset[sz];

	for (unsigned int x = 0; x < w; x++) {
	for (unsigned int y = 0; y < h; y++) {
		if (!lvl.blocked(x, y, z))
			continue;
		if (x < w - 1 && lvl.blocked(x+1, y, z))
			forest[x * h + y].join(forest[(x + 1) * h + y]);
		if (y < h - 1 && lvl.blocked(x, y+1, z) &&
			(y != h / 2 || forest[x * h + y].find() != forest[0].find()))
			forest[x * h + y].join(forest[x * h + y + 1]);
	}
	}

	std::vector<Component> cs;

	for (unsigned int x = 0; x < w; x++) {
	for (unsigned int y = 0; y < h; y++) {
		if (!lvl.blocked(x, y, z))
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

std::vector<Polygon> Lvl::polys(unsigned int z) const {
	std::vector<Component> comps = components(*this, z);
	std::vector<Polygon> polys;
	// 2.0 + 5.0 / 4.0 is from Player::runspeed()
//	double w = Tile::Width / (2.0 + 5.0 / 4.0);
//	double h = Tile::Height / Body::Maxdy;
	for (unsigned int i = 0; i < comps.size(); i++)
		polys.push_back(comps[i].poly(1, 1));
	return polys;
}