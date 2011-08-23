#include <cstdio>

class Tiles {
public:
	enum {
		Ntiles = WIDTH * HEIGHT,
		Width = WIDTH,
		Height = HEIGHT,
	};

	typedef unsigned int Tile;

	typedef unsigned int Pos;

	typedef unsigned int Cost;

	typedef int Oper;

	Tiles(FILE*);

protected:
	struct {
		int n;
		Pos mv[4];
	} ops[Ntiles];

	Tile init[Ntiles];
	Pos goalpos[Ntiles];

private:
	void readruml(FILE*);
};


class TilesMdist : Tiles {
public:
	class State {
		Cost h;
		Pos b;
		Tile ts[Ntiles];
		friend class TilesMdist;
	};

	struct Undo {
		Pos b;
		Cost h;
		friend class TilesMdist;
	};

	TilesMdist(FILE*);

	State initstate(void);

	Cost h(State *s) { return s->h; }

	unsigned int nops(State *s) { return ops[s->b].n; }

	Oper nthop(State *s, unsigned int n) { return ops[s->b].mv[n]; }

	void undoinfo(Undo *u, State *s, Oper op) {
		u->h = s->h;
		u->b = op;
	}

	void undo(State *s, Undo *u) {
		s->ts[s->b] = s->ts[u->b];
		s->b = u->b;
		s->h = u->h;
	}

	void apply(State *s, Oper op) {
		Tile t = s->ts[op];
		s->ts[s->b] = t;
		s->b = op;
		s->h += incr[t][op][s->b];
	}

	Cost mdist(Tile[]);

private:
	void initmd(void);
	void initincr(void);

	Cost md[Ntiles][Ntiles];
	Cost incr[Ntiles][Ntiles][Ntiles];
};