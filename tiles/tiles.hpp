#include <cstdio>
#include <cassert>

extern "C" unsigned long hashbytes(unsigned char[], unsigned int);

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
	typedef int Oper;

	Tiles(FILE*);

	static void dumptiles(FILE*, Tile []);

	static unsigned long hash(Tile ts[]) {
/*
		return hashbytes((unsigned char*) ts, sizeof(Tile)*Ntiles);
*/

		unsigned long h = 0;
		for (int i = 0; i < Ntiles; i++)
			h += hashvec[i][ts[i]];
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