#include "tiles.hpp"
#include <cstring>

class TilesMdist : Tiles {
public:

	typedef Tiles::Tile Tile;
	typedef Tiles::Pos Pos;

	typedef Tiles::Cost Cost;
	enum { InfCost = -1 };

	typedef int Oper;
	enum { Nop = -1 };

	class State {
		Cost h;
		Pos b;
		Tile ts[Ntiles];
		friend class TilesMdist;
	public:
		unsigned long hash(void) {
			return Tiles::hash(ts);
		}
		bool eq(State &b) const {
			for (int i = 0; i < Ntiles; i++) {
				if (ts[i] != b.ts[i])
					return false;
			}
			return true;
		}
	};

	class Undo {
		Pos b;
		Cost h;
		friend class TilesMdist;
	public:
		Undo(State &s, Oper op) {
			h = s.h;
			b = s.b;
		}
	};

	TilesMdist(FILE*);

	State initstate(void);

	Cost h(State &s) {
		return s.h;
	}

	bool isgoal(State &s) {
		return s.h == 0;
	}

	unsigned int nops(State &s) {
		return ops[s.b].n;
	}

	Oper nthop(State &s, unsigned int n) {
		assert (n < ops[s.b].n);
		return ops[s.b].mvs[n];
	}

	Oper revop(State &s, Oper op) {
		return s.b;
	}

	Cost opcost(State &s, Oper op) {
		return 1;
	}

	void undo(State &s, Undo &u) {
		s.ts[s.b] = s.ts[u.b];
		s.b = u.b;
		s.h = u.h;
	}

	void apply(State &s, Oper newb) {
		Tile t = s.ts[newb];
		s.ts[s.b] = t;
		s.h += incr[t][newb][s.b];
		s.b = newb;
	}

	void applyinto(State &cpy, State &s, Oper newb) {
		Tile t = s.ts[newb];
		cpy.h = s.h + incr[t][newb][s.b];
		cpy.b = newb;

		for (int i = 0; i < Ntiles; i++) {
			if (i == newb)
				cpy.ts[i] = 0;
			else if ((unsigned int) i == s.b)
				cpy.ts[i] = t;
			else
				cpy.ts[i] = s.ts[i];
		}
	}

	void dumpstate(FILE *out, State &s) {
		s.ts[s.b] = 0;
		Tiles::dumptiles(out, s.ts);
		fprintf(out, "h=%u\n", s.h);
	}

private:
	void initmd(void);
	void initincr(void);

	unsigned int md[Ntiles][Ntiles];
	int incr[Ntiles][Ntiles][Ntiles];
};