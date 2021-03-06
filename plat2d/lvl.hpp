// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#pragma once
#include "body.hpp"
#include "tile.hpp"
#include <cstdio>
#include <vector>

struct Image;

struct Lvl {

	// This constructor creates an empty level.
	Lvl(unsigned int, unsigned int);

	// This constructor loads a level from a file.
	Lvl(FILE*);

	~Lvl();

	// width returns the width of the level in number of blocks.
	unsigned int width() const { return w; }

	// height returns the height of the level in number of
	// blocks.
	unsigned int height() const { return h; }

	// draw draws the level on an image.
	void draw(Image&) const;

	// isect returns intersection information for a bounding
	// box moving at a given velocity through the level.
	Isect isect(const Bbox&, const geom2d::Pt&) const;

	struct Blk {
		Blk() : tile(0) { }
		unsigned int tile;
	};

	// Blkinfo contains information on a specific level block.
	struct Blkinfo {
		Blkinfo(const Blk &b, unsigned int i, unsigned int j) :
			blk(b), tile(tiles[b.tile]), x(i), y(j) { }

		const Blk &blk;
		const Tile &tile;
		unsigned int x, y;
	};

	// blocked returns true if the level block at the given coordinate
	// is collidable.
	bool blocked(unsigned int x, unsigned int y) const {
		return tiles[blks[ind(x, y)].tile].flags & Tile::Collide;
	}

	// at returns the block information structure for the
	// level block at the given grid coordinate.
	Blkinfo at(unsigned int x, unsigned int y) const {
		return Blkinfo(blks[ind(x, y)], x, y);
	}

	// majorblk returns the block information struct for the level
	// block that contains a majority of the rectangle.
	Blkinfo majorblk(const Bbox &r) const {
		const geom2d::Pt &c = r.center();
		return at(c.x / Tile::Width, c.y / Tile::Height);
	}

private:

	void read(FILE*);

	void readtile(FILE*, unsigned int, unsigned int);

	unsigned int ind(unsigned int x, unsigned int y) const {
		return x * h + y;
	}

	Blk *blk(unsigned int x, unsigned int y) {
		return blks + ind(x, y);
	}

	unsigned int w, h;
	Blk *blks;
};