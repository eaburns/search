#ifndef _TILES_HPP_
#define _TILES_HPP_

#include <cstdio>
#include <cassert>

class Tiles {
public:
	enum {
		Ntiles = WIDTH * HEIGHT,
		Width = WIDTH,
		Height = HEIGHT,
	};

	typedef unsigned int Tile;
	typedef unsigned int Pos;

	typedef int Cost;
	enum { InfCost = -1 };

	typedef int Oper;
	enum { Nop = -1 };

	Tiles(FILE*);

	static void dumptiles(FILE*, Tile []);

	static unsigned long hash(Tile ts[], Pos b) {
		unsigned long h = 0;
		for (unsigned int i = 0; i < Ntiles; i++) {
			if (i == b)
				continue;
			h ^= hashvec[i][ts[i]];
		}
		return h;

	}

	static unsigned long korf_hash(Tile ts[], Pos b) {
		unsigned long h = ts[0];
		for (int i = 1; i < Ntiles; i++)
			h += h * 3 + ts[i];
		return h;
	}

protected:

	struct {
		unsigned int n;
		Pos mvs[4];
	} ops[Ntiles];

	Tile init[Ntiles];
	Pos goalpos[Ntiles];
private:
	void readruml(FILE*);
	void initops(void);

	void inithashvec(void);
	static bool hashvecinit;
	static unsigned long hashvec[Ntiles][Ntiles];
};

#endif	// _TILES_HPP_
