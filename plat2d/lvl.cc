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
	Hitzone(Rect a, const Point &v) {
		Rect b(a);
		b.move(v.x, v.y);

		x0 = 0.5 + a.a.x / (double) Tile::Width;
		y0 = 0.5 + a.a.y / (double) Tile::Height;
		x1 = 0.5 + ceil(a.b.x) / (double) Tile::Width;
		y1 = 0.5 + ceil(a.b.y) / (double) Tile::Height;
		if (y0 > 0)
			y0--;
	}

	unsigned int x0, y0, x1, y1;
};

Isect Lvl::isection(unsigned int z, const Rect &r, const Point &v) const {
	Hitzone test(r, v);
	Isect isect;
	Rect mv(r);

	printf("hitzone: x=%u→%u, y=%u→%u\n", test.x0, test.x1, test.y0, test.y1);

	for (unsigned int x = test.x0; x <= test.x1; x++) {
	for (unsigned int y = test.y0; y <= test.y1; y++) {
		unsigned int t = blks[ind(x, y, z)].tile;
		Isect is(tiles[t].isection(x, y, mv));
		if (is.is && is.dy > isect.dy) {
			printf("isect!, x=%u, y=%u\n", x, y);
			isect.is = true;
			isect.dy = is.dy;
		}
	}
	}

	if (v.x == 0.0)
		return isect;

	mv = r;
	mv.move(v.x, v.y + (v.y < 0 ? isect.dy : -isect.dy));

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