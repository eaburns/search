#pragma once

#include <cstdio>
#include <cassert>
#include "body.hpp"
#include "../utils/image.hpp"

struct Tile {
	enum {
		Collide = 1<<0,
		Water = 1<<1,
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

	Tile() : c(0) { }

	Tile(char c, unsigned int f) : c(c), flags(f) { }

	Isect isect(unsigned int x, unsigned int y, const Bbox & r) const {
		if (!(flags & Collide))
			return Isect();	
		return r.isect(bbox(x, y));
	}

	double gravity() const {
		static const double Grav = 0.5;
		return flags & Water ? 0.5 * Grav : Grav;
	}

	double drag() const {
		return flags & Water ? 0.7 : 1.0;
	}

	char c;	// c == 0 is invalid
	unsigned int flags;
};

struct Tiles {

	Tiles();

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
