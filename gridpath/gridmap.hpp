#ifndef _GRIDMAP_HPP_
#define _GRIDMAP_HPP_

#include <climits>
#include <cstdio>
#include <string>

void fatal(const char*, ...);

class GridMap {
public:
	GridMap(std::string &file);
	~GridMap(void);

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

	unsigned int x(unsigned int loc) const { return loc % w; }

	unsigned int y(unsigned int loc) const { return loc / w; }

	unsigned int loc(unsigned int x, unsigned int y) const { return y * w + x; }

	unsigned int width(void) const { return w; }

	unsigned int height(void) const { return h; }

	unsigned int up(unsigned int l) const { return l - w; }

	unsigned int down(unsigned int l) const { return l + w; }

	unsigned int left(unsigned int l) const { return l - 1; }

	unsigned int right(unsigned int l) const { return l + 1; }

	void output(FILE*) const;

	std::string &filename(void) { return file; }
private:

	void load(FILE*);

	enum {
		Passable = 1 << 0,
		OutOfBounds = 1 << 1,
		Tree = 1 << 2,
		Swamp = 1 << 3,
		Water = 1 << 4,
	};

	const struct Terrain{
		char flags[CHAR_MAX];
		Terrain() {
			flags['.'] = Passable;
			flags['G'] = Passable;
			flags['@'] = OutOfBounds;
			flags['O'] = OutOfBounds;
	 		flags['T'] = Tree;
			flags['S'] = Swamp;
			flags['W'] = Water;
		}
	} terrain;

	enum { Bufsz = 128 };

	char typ[Bufsz];
	unsigned int w, h;
	unsigned char *map;
	unsigned char *flags;
	std::string file;
};

#endif	// _GRIDMAP_HPP_
