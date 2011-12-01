#include "../utils/utils.hpp"
#include "player.hpp"
#include "lvl.hpp"
#include <cstdio>

struct Plat2d {

	static const unsigned int Ops[];
	static const unsigned int Nops;

	enum { UnitCost = true };

	typedef int Cost;
	static const int InfCost = -1;

	typedef int Oper;
	static const int Nop = -1;

	Plat2d(FILE*);

	struct State {
		State(void) { }

		State(unsigned int x, unsigned int y, unsigned int z,
			unsigned int w, unsigned int h) : player(x, y, z, w, h) { }

		// hash does nothing since the hash table
		// for plat2d states doesn't store anything.
		unsigned long hash(void) { return -1; }

		bool eq(State &) const {
			fatal("Unimplemented");
			return false;
		}

		Player player;
	};

	typedef State PackedState;

	struct Undo {
		Undo(State&, Oper) { }
	};

	State initialstate(void);

	Cost h(State &s) {
		return 0;
	}

	Cost d(State &s) {
		return 0;
	}

	bool isgoal(State &s) {
		return false;
	}

	unsigned int nops(State &s) {
		return Nops;
	}

	Oper nthop(State &s, unsigned int n) {
		return Ops[n];
	}

	Oper revop(State &s, Oper op) {
		return Nop;
	}

	Cost opcost(State &s, Oper op) {
		return 1;
	}

	void undo(State &s, Undo &u) { }

	State &apply(State &buf, State &s, Oper op) {
		assert (op != Nop);
		buf = s;
		buf.player.act(lvl, (unsigned int) op);
		return buf;
	}

	void pack(PackedState &dst, State &src) {
		dst = src;
	}

	State &unpack(State &buf, PackedState &pkd) {
		return pkd;
	}

	void dumpstate(FILE *out, State &s) {
		fprintf(out, "%g, %g\n", s.player.loc().x, s.player.loc().y);
	}

private:
	Lvl lvl;
};