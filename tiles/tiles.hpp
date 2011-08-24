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


class TilesMdist : Tiles {
public:

	typedef Tiles::Tile Tile;

	typedef Tiles::Pos Pos;

	typedef Tiles::Cost Cost;

	enum { InfCost = -1 };
	enum { UnitCost = true };

	typedef int Oper;

	enum { Nop = -1 };

	class State {
		Cost h;
		Pos b;
		Tile ts[Ntiles];
		friend class TilesMdist;
	};

	class Undo {
		Pos b;
		Cost h;
		friend class TilesMdist;
	};

	TilesMdist(FILE*);

	State initstate(void);

	Cost h(State *s) {
		return s->h;
	}

	bool isgoal(State *s) {
		return s->h == 0;
	}

	unsigned int nops(State *s) {
		return ops[s->b].n;
	}

	Oper nthop(State *s, unsigned int n) {
		assert (n < ops[s->b].n);
		return ops[s->b].mvs[n];
	}

	Oper revop(State *s, Oper op) {
		return s->b;
	}

	Cost opcost(State *s, Oper op) {
		return 1;
	}

	void undoinfo(Undo &u, State *s, Oper op) {
		u.h = s->h;
		u.b = s->b;
	}

	void undo(State *s, Undo &u) {
		s->ts[s->b] = s->ts[u.b];
		s->b = u.b;
		s->h = u.h;
	}

	void apply(State *s, Oper newb) {
		Tile t = s->ts[newb];
		s->ts[s->b] = t;
		s->h += incr[t][newb][s->b];
		s->b = newb;
	}

	void dumpstate(FILE *out, State *s) {
		s->ts[s->b] = 0;
		Tiles::dumptiles(out, s->ts);
		fprintf(out, "h=%u\n", s->h);
	}

private:
	void initmd(void);
	void initincr(void);

	unsigned int md[Ntiles][Ntiles];
	int incr[Ntiles][Ntiles][Ntiles];
};