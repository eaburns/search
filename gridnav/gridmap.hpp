#ifndef _GRIDMAP_HPP_
#define _GRIDMAP_HPP_

#include <climits>
#include <cstdio>
#include <string>

void fatal(const char*, ...);

class GridMap {
public:
	GridMap(std::string &file);

	GridMap(FILE *f) { load(f); }

	~GridMap(void);

	unsigned int width(void) const { return w; }

	unsigned int height(void) const { return h; }

	unsigned int size(void) const { return sz; }

	unsigned int x(unsigned int loc) const { return loc % w; }

	unsigned int y(unsigned int loc) const { return loc / w; }

	bool passable(unsigned int loc) const { return flags[loc] & Passable; }

	unsigned int loc(unsigned int x, unsigned int y) const { return y * w + x; }

	unsigned int up(unsigned int l) const { return l - w; }

	unsigned int down(unsigned int l) const { return l + w; }

	unsigned int left(unsigned int l) const { return l - 1; }

	unsigned int right(unsigned int l) const { return l + 1; }

	unsigned int ops(unsigned int l, int ops[]) {
		switch (mtype) {
		case Octile: return octileops(l, ops);
		case EightWay: return eightwayops(l, ops);
		case FourWay: return fourwayops(l, ops);
		}
		fatal("Invalid movement type: %d", mtype);
		return 0;	// Unreachable
	}

	void output(FILE*) const;

	std::string &filename(void) { return file; }

	enum {
		Octile,
		EightWay,
		FourWay,
	};

	int movetype(void) { return mtype; }

private:

	unsigned int octileops(unsigned int l, int ops[]) {
		unsigned int n = 0;

		bool leftok = x(l) > 0 && terrainok(l, left(l));
		if (leftok)
			ops[n++] = left(l);

		bool rightok = x(l) < w - 1 && terrainok(l, right(l));
		if (rightok)
			ops[n++] = right(l);

		bool upok = y(l) > 0 && terrainok(l, up(l));
		if (upok) {
			ops[n++] = up(l);
			if (leftok && terrainok(l, up(left(l))))
				ops[n++] = up(left(l));
			if (rightok && terrainok(l, up(right(l))))
				ops[n++] = up(right(l));
		}

		bool downok = y(l) < h - 1 && terrainok(l, down(l));
		if (downok) {
			ops[n++] = down(l);
			if (leftok && terrainok(l, down(left(l))))
				ops[n++] = down(left(l));
			if (rightok && terrainok(l, down(right(l))))
				ops[n++] = down(right(l));
		}

		return n;
	}

	unsigned int eightwayops(unsigned int l, int ops[]) {
		unsigned int n = 0;

		if (x(l) > 0 && terrainok(l, left(l)))
			ops[n++] = left(l);
		if (x(l) < w - 1 && terrainok(l, right(l)))
			ops[n++] = right(l);

		if (y(l) > 0) {
			if (terrainok(l, up(l)))
				ops[n++] = up(l);
			if (x(l) > 0 && terrainok(l, up(left(l))))
				ops[n++] = up(left(l));
			if (x(l) < w - 1 && terrainok(l, up(right(l))))
				ops[n++] = up(right(l));
		}

		if (y(l) < h - 1) {
			if (terrainok(l, down(l)))
				ops[n++] = down(l);
			if (x(l) > 0 && terrainok(l, down(left(l))))
				ops[n++] = down(left(l));
			if (x(l) < w - 1 && terrainok(l, down(right(l))))
				ops[n++] = down(right(l));
		}

		return n;
	}

	unsigned int fourwayops(unsigned int l, int ops[]) {
		unsigned int n = 0;
		if (x(l) > 0 && terrainok(l, left(l)))
			ops[n++] = left(l);
		if (x(l) < w - 1 && terrainok(l, right(l)))
			ops[n++] = right(l);
		if (y(l) > 0 && terrainok(l, up(l)))
			ops[n++] = up(l);
		if (y(l) < h - 1 && terrainok(l, down(l)))
			ops[n++] = down(l);
		return n;
	}

	// Tests whether the terrain flags allow this move.
	bool terrainok(unsigned int l0, unsigned int l1) const {
		char f0 = flags[l0];
		char f1 = flags[l1];

		if (f1 & (OutOfBounds | Tree))
			return false;

		if (f0 & Water)
			return f1 & Water;

		return f1 & (Passable | Swamp);
	}

	void readfail(const char*, ...);

	void load(FILE*);

	void load_ruml(FILE*);

	void load_sturtevant(FILE*);

	enum {
		Passable = 1 << 0,
		OutOfBounds = 1 << 1,
		Tree = 1 << 2,
		Swamp = 1 << 3,
		Water = 1 << 4,
	};

	int mtype;
	unsigned int w, h, sz;
	unsigned char *map;
	unsigned char *flags;
	std::string file;
};

#endif	// _GRIDMAP_HPP_
