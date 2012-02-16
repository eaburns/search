#ifndef _GRIDMAP_HPP_
#define _GRIDMAP_HPP_

#include <vector>
#include <climits>
#include <cstdio>
#include <string>

void fatal(const char*, ...);

class GridMap {
public:
	GridMap(std::string &file);

	GridMap(FILE *f) : nmoves(0) { load(f); }

	~GridMap(void);

	unsigned int xcoord(unsigned int loc) const { return loc % w; }

	unsigned int ycoord(unsigned int loc) const { return loc / w; }

	unsigned int loc(unsigned int x, unsigned int y) const { return y * w + x; }

	bool passable(unsigned int loc) const { return flags[loc] & Passable; }

	struct Move {
		Move(void) : n(0) { }

		Move(const GridMap&, int, int, unsigned int, ...);

		int dx, dy, delta;
		float cost;
		unsigned int n;
		struct { int dx, dy, delta; } chk[3];
	};

	bool ok(unsigned int loc, const Move &m) const {
		int x = xcoord(loc), y = ycoord(loc);
		for (unsigned int i = 0; i < m.n; i++) {
			int dx = m.chk[i].dx, dy = m.chk[i].dy;
			if (x + dx < 0 || x + dx >= (int) w || y + dy < 0 || y + dy >= (int) h ||
					!terrainok(loc, loc + m.chk[i].delta))
				return false;
		}
		return true;
	}

	unsigned int w, h, sz;
	unsigned char *map;
	std::string file;

	unsigned int nmoves;
	Move moves[8];

private:
	enum {
		Passable = 1 << 0,
		OutOfBounds = 1 << 1,
		Tree = 1 << 2,
		Swamp = 1 << 3,
		Water = 1 << 4,
	};

	// Tests whether the terrain flags allow this move.
        bool terrainok(unsigned int l0, unsigned int l1) const {
                char f0 = flags[l0], f1 = flags[l1];
                if (f1 & (OutOfBounds | Tree))
                        return false;
                else if (f0 & Water)
                        return f1 & Water;
                return f1 & (Passable | Swamp);
        }

	unsigned char *flags;

	void readfail(const char*, ...);
	void load(FILE*);
	void load_ruml(FILE*);
	void load_sturtevant(FILE*);
	void octile(void);
	void eightway(void);
	void fourway(void);
};

#endif	// _GRIDMAP_HPP_
