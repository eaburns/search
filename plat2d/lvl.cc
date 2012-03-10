#include "lvl.hpp"
#include "tile.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"
#include <cstring>
#include <cmath>
#include <cerrno>
#include <limits>

Lvl::Lvl(unsigned int _w, unsigned int _h) : w(_w), h(_h) {
	if (w > MaxDim || h > MaxDim)
		fatal("Level is too large");

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
	unsigned int d, ignore;
	int ret = fscanf(f, " %u %u %u %u",&d, &w, &h, &ignore);
	if (ret == EOF && ferror(f))
		fatal("Failed to read the level");
	if (ret != 4)
		fatal("Malformed level header");
	if (d != 1)
		fatal("Only levels with a depth of 1 are supported");

	if (w > MaxDim || h > MaxDim || d > MaxDim)
		fatal("Level is too large");

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
	Hitzone(Bbox a, const Geom2d::Pt &v) {
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

Isect Lvl::isect(const Bbox &r, const Geom2d::Pt &v) const {
	Hitzone test(r, v);
	Isect isect;
	Bbox mv(r);

	if (v.y == 0.0)
		goto testx;

	mv.translate(0, v.y);
	for (unsigned int x = test.x0; x <= test.x1; x++) {
	for (unsigned int y = test.y0; y <= test.y1; y++) {
		unsigned int t = blks[ind(x, y)].tile;
		Isect is(tiles[t].isect(x, y, mv));
		if (is.is && is.dy > isect.dy) {
			isect.is = true;
			isect.dy = is.dy;
		}
	}
	}

testx:
	if (v.x == 0.0)
		return isect;

	mv = r;
	mv.translate(v.x, v.y + (v.y < 0 ? isect.dy : -isect.dy));

	for (unsigned int x = test.x0; x <= test.x1; x++) {
	for (unsigned int y = test.y0; y <= test.y1; y++) {
		unsigned int t = blks[ind(x, y)].tile;
		Isect is(tiles[t].isect(x, y, mv));
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
			double rot = M_PI / 2;
			if (tiles[t].flags & Tile::Down)
				rot = 3 * M_PI / 2;
			Geom2d::Pt c = Geom2d::Pt(xpos, ypos);
			Geom2d::Poly t = Geom2d::Poly::triangle(c, Tile::Width, M_PI/4, rot);
			img.add(new Image::Poly(t, Image::black, 1));
		}
	}
	}
}
