#ifndef _GRIDMAP_HPP_
#define _GRIDMAP_HPP_

#include <vector>
#include <climits>
#include <cstdio>
#include <string>

void fatal(const char*, ...);

struct GridMap {

	GridMap(std::string &file);

	GridMap(FILE *f) : nmvs(0) { load(f); }

	~GridMap(void);

	// coord returns x,y coordinate for the given array index.
	std::pair<int,int> coord(int loc) const { return std::pair<int,int>(loc%w, loc / w); }

	// loc returns the array index for the x,y coordinate.
	int index(int x, int y) const { return y * w + x; }

	// blkd returns true if the given location is blocked.
	bool blkd(int l) const { return !(flags[l] & Passable); }

	struct Move {
		Move(void) : n(0) { }

		Move(const GridMap&, int, int, unsigned int, ...);

		int dx, dy, delta;
		double cost;
		unsigned int n;
		struct { int dx, dy, delta; } chk[3];
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

	unsigned int w, h, sz;
	unsigned char *map;
	std::string file;

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

	// octile computes octile grid operators. Octile operators
	// disallow diagonal movements unless the two adjacent
	// cells are also unblocked.
	void octile(void);

	// eightway computes eight-way grid operators.  Eight-way
	// operators allow diagonal even if the adjacent cells are blocked.
	void eightway(void);

	// fourway computes four-way grid operators.
	void fourway(void);
};

#endif	// _GRIDMAP_HPP_
