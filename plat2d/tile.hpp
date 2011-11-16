#ifndef _TILE_HPP_
#define _TILE_HPP_

#include <cstdio>
#include <cassert>
#include "geom.hpp"

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
		Width = 64,
		Height = 64,
	};

	static int read(FILE*);

	static Rect bbox(unsigned int x, unsigned int y) {
		return Rect(x * Width, y * Height,
			(x + 1) * Width, (y + 1) * Height);
	}

	Tile(void) : ok(false) { }

	Tile(unsigned int f) : ok(true), flags(f) { }

	Isect isection(unsigned int x, unsigned int y, const Rect & r) const {	
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

	bool ok;
	unsigned int flags;
};

struct Tiles {

	Tiles(void);

	bool istile(int t) const {
		return t >= 0 && t < Ntiles && tiles[t].ok;
	}

	const Tile &operator[](unsigned int i) const {
		assert (tiles[i].ok);
		return tiles[i];
	}

	enum { Ntiles = 256 };

private:
	Tile tiles[Ntiles];
};

extern const Tiles tiles;

#endif	// _TILE_HPP_