#ifndef _TILE_HPP_
#define _TILE_HPP_

#include <cstdio>
#include <cassert>
#include "body.hpp"
#include "../utils/image.hpp"

struct Tile {
	enum {
		Collide = 1<<0,
		Water = 1<<1,
		Fdoor = 1<<2,
		Bdoor = 1<<3,
		Down = 1 << 4,
		Up = 1 << 5,
		Opaque = 1 << 6,
	};

	enum {
		Width = 32,
		Height = 32,
	};

	static int read(FILE*);

	static Bbox bbox(unsigned int x, unsigned int y) {
		return Bbox(x * Width, y * Height,
			(x + 1) * Width, (y + 1) * Height);
	}

	static void draw(Image&, unsigned int, unsigned int, Color);

	Tile(void) : c(0) { }

	Tile(char _c, unsigned int f) : c(_c), flags(f) { }

	Isect isection(unsigned int x, unsigned int y, const Bbox & r) const {
		if (!(flags & Collide))
			return Isect();	
		return r.isection(bbox(x, y));
	}

	double gravity(void) const {
		static const double Grav = 0.5;
		return flags & Water ? 0.5 * Grav : Grav;
	}

	double drag(void) const {
		return flags & Water ? 0.7 : 1.0;
	}

	char c;	// c == 0 is invalid
	unsigned int flags;
};

struct Tiles {

	Tiles(void);

	bool istile(int t) const {
		return t >= 0 && t < Ntiles && tiles[t].c != 0;
	}

	const Tile &operator[](unsigned int i) const {
		assert (tiles[i].c != 0);
		assert ((unsigned int) tiles[i].c == i);
		return tiles[i];
	}

	enum { Ntiles = 256 };

private:
	Tile tiles[Ntiles];
};

extern const Tiles tiles;

#endif	// _TILE_HPP_