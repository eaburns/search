#include <cstring>
#include "tiles.hpp"
#include "packed.hpp"

class TilesMdist : public Tiles {
public:
	typedef PackedTiles<Ntiles> PackedState;

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
				if (ts[i] != other.ts[i])
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

	State &apply(State &buf, State &s, Oper newb) {
		Tile t = s.ts[newb];
		s.ts[s.b] = t;
		s.h += incr[t][newb][s.b];
		s.b = newb;
		return s;
	}

	void pack(PackedState &dst, State &s) {
		s.ts[s.b] = 0;
		dst.pack(s.ts);
	}

	State &unpack(State &buf, PackedState &pkd) {
		buf.b = pkd.unpack_md(md, buf.ts, &buf.h);
		return buf;
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
