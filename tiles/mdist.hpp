#include <cstring>
#include "tiles.hpp"
#include "packed.hpp"

class TilesMdist : public Tiles {
public:

	typedef PackedTiles<Ntiles> PackedState;

	struct State {
		bool eq(State &other) const {
			if (b != other.b)
				return false;
			for (unsigned int i = 0; i < Ntiles; i++) {
				if (ts[i] != other.ts[i] && i != b)
					return false;
			}
			return true;
		}
	private:
		friend class TilesMdist;
		Tile ts[Ntiles];
		Pos b;
		Cost h;
	};

	TilesMdist(FILE*);

	State initialstate(void);

	unsigned long hash(PackedState &p) {
		return p.hash();
	}

	Cost h(State &s) const { return s.h; }

	Cost d(State &s) const { return s.h; }

	bool isgoal(State &s) const { return s.h == 0; }

	unsigned int nops(State &s) const { return ops[s.b].n; }

	Oper nthop(State &s, unsigned int n) const {
		assert (n < ops[s.b].n);
		return ops[s.b].mvs[n];
	}

	struct Edge {
		Cost cost;
		Oper revop;
		State &state;

		Edge(TilesMdist &d, State &s, Oper op) :
				cost(1), revop(s.b), state(s), oldh(s.h) {
			Tile t = state.ts[op];
			state.ts[state.b] = t;
			state.h += d.incr[t][op][state.b];
			state.b = op;
		}

		~Edge(void) {
			state.ts[state.b] = state.ts[revop];
			state.b = revop;
			state.h = oldh;
		}

	private:
		friend class TilesMdist;
		Cost oldh;
	};

	void pack(PackedState &dst, State &s) const {
		s.ts[s.b] = 0;
		dst.pack(s.ts);
	}

	State &unpack(State &buf, PackedState &pkd) const {
		buf.b = pkd.unpack_md(md, buf.ts, &buf.h);
		return buf;
	}

	void dumpstate(FILE *out, State &s) const {
		s.ts[s.b] = 0;
		Tiles::dumptiles(out, s.ts);
	}

protected:

	unsigned int md[Ntiles][Ntiles];

private:
	void initmd(void);
	void initincr(void);

	int incr[Ntiles][Ntiles][Ntiles];
};
