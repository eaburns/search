#pragma once

#include <vector>
#include <climits>
#include <cstdio>
#include <string>

void fatal(const char*, ...);

struct GridMap {

	// Creates an empty four-way unit cost grid of the
	// given dimensions.
	GridMap(unsigned int w, unsigned int h);

	GridMap(std::string &file);

	GridMap(FILE *f) : nmvs(0) { load(f); }

	// uniform returns a GridMap that has uniformly
	// distributed obstacles.
	static GridMap uniform(unsigned int w, unsigned int h,
		double prob, uint64_t seed);

	~GridMap();

	// coord returns x,y coordinate for the given array index.
	std::pair<int,int> coord(int loc) const {
		return std::pair<int,int>(loc%w, loc / w);
	}

	// ycoord returns y coordinate for the given array index.
	int ycoord(int loc) const {
		return loc / w;
	}

	// loc returns the array index for the x,y coordinate.
	int index(int x, int y) const {
		return y * w + x;
	}

	// blkd returns true if the given location is blocked.
	bool blkd(int l) const {
		return !(flags[l] & Passable);
	}

	// A Move contains information for a single
	// possible move to and adjacent cell of the
	// grid.
	struct Move {
		Move() : n(0) { }

		Move(const GridMap&, int, int, unsigned int, ...);

		// dx, dy give the x and y displacement of
		// this move.
		int dx, dy;

		// delta gives the loc displacement of
		// this move.
		int delta;

		// cost is √2 for diagonal moves and 1 for
		// both vertical and horizontal moves.
		double cost;

		// chk array contains the displacements
		// for cells that must be unblocked in order
		// for this move to be valid.
		struct { int dx, dy, delta; } chk[3];

		// n is the number of valid elements in
		// the chk array.
		unsigned int n;
	};

	// ok returns true if the given move is valid from the
	// given location.
	bool ok(int loc, const Move &m) const {
		for (unsigned int i = 0; i < m.n; i++) {
			int nxt = loc + m.chk[i].delta;
			if (!flagsok(loc, nxt))
				return false;
		}
		return true;
	}

	// setoctile sets mvs to octile movement. Octile moves
	// disallow diagonal movements unless the two adjacent
	// cells are also unblocked.
	void setoctile();

	// seteightway sets mvs to eight-way movement.  Eight-way
	// moves allow diagonal even if the adjacent cells are blocked.
	void seteightway();

	// setfourway sets mvs to four-way movements.
	void setfourway();

	unsigned int w, h, sz;
	unsigned char *map;
	std::string file;

	// lifecost is true for `life cost' grid maps.  In
	// these maps, the cost of each move should
	// be multiplied by the y coordinate of the
	// cell from which the move is taking place.
	bool lifecost;

	unsigned int nmvs;
	Move mvs[8];

private:

	enum {
		Passable = 1 << 0,
		OutOfBounds = 1 << 1,
		Tree = 1 << 2,
		Swamp = 1 << 3,
		Water = 1 << 4,
	};

	// flagsok returns true if the flags allow a move from l0 to l1.
        bool flagsok(int l0, int l1) const {
		char f0 = flags[l0], f1 = flags[l1];
		if (f1 & (OutOfBounds | Tree))
			return false;
		else if (f0 & Water)
			return f1 & Water;
		return f1 & (Passable | Swamp);
        }

	unsigned char *flags;

	// readfail prints a failure message when loading the grid is not
	// successful.
	void readfail(const char*, ...);

	// load attempts to figure out if the map is a Ruml or Sturtevant map
	// and then loads it.
	void load(FILE*);

	// load_seedinst loads a Wheeler-style map
	// that is described by its seed.
	void load_seedinst(FILE*);

	// load_ruml loads one of Wheeler's map
	void load_ruml(FILE*);

	// load_sturtevant loads one of Nathan's maps from movingai.org
	void load_sturtevant(FILE*);

	// setsize allocates the map and flags arrays and sets
	// w, h and sz.  The size of the map is one extra row/col
	// on each side set to OutOfBounds.
	void setsize(unsigned int, unsigned int);
};
