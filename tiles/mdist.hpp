#include <cstring>
#include <boost/integer/static_log2.hpp>
#include "tiles.hpp"

class TilesMdist : Tiles {
public:

	typedef Tiles::Tile Tile;
	typedef Tiles::Pos Pos;

	typedef Tiles::Cost Cost;
	enum { InfCost = -1 };

	typedef int Oper;
	enum { Nop = -1 };

	class State {
		friend class TilesMdist;
		Tile ts[Ntiles];
		Pos b;
		Cost h;
	public:
		unsigned long hash(void) {
			return Tiles::hash(ts, b);
		}
		bool eq(State &other) const {
			if (b != other.b)
				return false;
			for (unsigned int i = 0; i < Ntiles; i++) {
				if (i != b && ts[i] != other.ts[i])
					return false;
			}
			return true;
		}
	};

	class PackedState {
		friend class TilesMdist;
		static const unsigned int nbits = boost::static_log2<Ntiles>::value;
		static const unsigned int nbytes = (int) ((nbits / 8.0) * Ntiles);

		unsigned char bytes[nbytes];
	public:
		unsigned long hash(void) {
			return hashbytes(bytes, sizeof(bytes));
		}
		bool eq(PackedState &other) const {
			return memcmp(bytes, other.bytes, sizeof(bytes)) == 0;
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
	}

private:
	void initmd(void);
	void initincr(void);

	unsigned int md[Ntiles][Ntiles];
	int incr[Ntiles][Ntiles][Ntiles];
};