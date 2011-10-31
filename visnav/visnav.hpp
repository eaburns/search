#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include <cstdio>

struct Visnav {
	enum { UnitCost = false };

	typedef double Cost;
	static const double InfCost = -1;
	typedef int Oper;
	static const int Nop = -1;

	Visnav(FILE*);

	struct Undo;

	struct State {
		State(int v) : vert(v) { }

		unsigned long hash(void) { return vert; }

		bool eq(State &other) const { return vert == other.vert; }
		
	private: 
		friend class Undo;
		friend class Visnav;
		int vert;
	};

	typedef State PackedState;

	struct Undo {
		Undo(State &s, Oper op) { s.vert = op; }
	};

	State initialstate(void);

	Cost h(State &s) {
		fatal("Unimplemented");
		return 0.0;
	}

	bool isgoal(State &s) {
		fatal("Unimplemented");
		return false;
	}

	unsigned int nops(State &s) {
		fatal("Unimplemented");
		return 0;
	}

	Oper nthop(State &s, unsigned int n) {
		fatal("Unimplemented");
		return 0;
	}

	Oper revop(State &s, Oper op) {
		fatal("Unimplemented");
		return 0;
	}

	Cost opcost(State &s, Oper op) {
		fatal("Unimplemented");
		return 0;
	}

	void undo(State &s, Undo &u) { }

	State &apply(State &buf, State &s, Oper op) {
		fatal("Unimplemented");
		return s;
	}

	void pack(PackedState &dst, State &src) {
		fatal("Unimplemented");
	}

	State &unpack(State &buf, PackedState &pkd) {
		fatal("Unimplemented");
		return buf;
	}

	void dumpstate(FILE *out, State &s) {
		fatal("Unimplemented");
	}
private:
};