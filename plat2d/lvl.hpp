#include "geom.hpp"
#include "tile.hpp"
#include <cstdio>

struct Image;

struct Lvl {

	Lvl(unsigned int, unsigned int, unsigned int);

	Lvl(FILE*);

	~Lvl(void);

	unsigned int width(void) const { return w; }

	unsigned int height(void) const { return h; }

	unsigned int depth(void) const { return d; }

	void draw(Image&) const;

	Isect isection(unsigned int, const Rect&, const Point&) const;

	struct Blk { unsigned int tile; };

	const Blk &majorblk(unsigned int z, Rect r) const {
		r.normalize();
		double xmid = (r.b.x + r.a.x) / 2;
		double ymid = (r.b.y + r.a.y) / 2;
		return blks[ind(xmid / Tile::Width, ymid / Tile::Height, z)];
	}

private:

	void read(FILE*);

	void readtile(FILE*, unsigned int, unsigned int, unsigned int);

	unsigned int ind(unsigned int x, unsigned int y, unsigned int z) const {
		return z * w * h + y * w + x;
	}

	Blk *blk(unsigned int x, unsigned int y, unsigned int z) {
		return blks + ind(x, y, z);
	}

	unsigned int w, h, d;

	Blk *blks;
};