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
	typedef int Oper;

	Tiles(FILE*);
	static void dumptiles(FILE*, Tile []);

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
};