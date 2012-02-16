#ifndef _GRIDPATH_HPP_
#define _GRIDPATH_HPP_

#include "gridmap.hpp"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>

class GridNav {
public:

	enum { UnitCost = false };

	typedef double Cost;
	enum { InfCost = -1 };

	typedef int Oper;	// Index into the ops arrays.
	enum { Nop = -1 };

	GridNav(GridMap*, unsigned int, unsigned int,
		unsigned int, unsigned int);

	struct State {
		State &operator=(const State &o) {
 			loc = o.loc;
			nops = -1;
 			return *this;
		}

		State(void) :nops(-1) { }

		State (const State &o) : loc(o.loc), nops(-1) { }

		State(unsigned int l) : loc(l), nops(-1) { }

	private:
		friend class GridNav;
		int loc;
		int nops;
		Oper ops[8];
	};

	struct PackedState {
		int loc;
		unsigned long hash(void) { return loc; }

		bool eq(const PackedState &other) const {
			return other.loc == loc;
		}
	};

	State initialstate(void);

	Cost h(State &s) const { return octiledist(s.loc, finish); }

	Cost d(State &s) const { return octilecells(s.loc, finish); }

	bool isgoal(State &s) const { return s.loc == finish; }

	unsigned int nops(State &s) const {
		if (s.nops > 0)
			return s.nops;

		s.nops = 0;
		for (unsigned int i = 0; i < map->nmoves; i++) {
			if (map->ok(s.loc, map->moves[i]))
				s.ops[s.nops++] = s.loc + map->moves[i].delta;
		}
		return s.nops;
	}

	Oper nthop(State &s, unsigned int n) const { return s.ops[n]; }

	struct Transition {
		Cost cost;
		Oper revop;
		State state;

		Transition(GridNav &d, State &s, Oper op) :
			cost(digaonal(d, s.loc, op) ? sqrtf(2.0) : 1.0),
			revop(s.loc), state(op) { }

	private:
		static bool digaonal(const GridNav &d, int l0, int l1) {
			return d.map->xcoord(l0) != d.map->xcoord(l1) && d.map->ycoord(l0) != d.map->ycoord(l1);
		}
	};

	void pack(PackedState &dst, State &src) const { dst.loc = src.loc; }

	State &unpack(State &buf, PackedState &pkd) const {
		buf.loc = pkd.loc;
		return buf;
	}

	void dumpstate(FILE *out, State &s) const {
		fprintf(out, "%u, %u\n", map->xcoord(s.loc), map->ycoord(s.loc));
	}

	int start, finish;
	GridMap *map;

private:

	float octiledist(int l0, int l1) const {
		int dx = abs(map->xcoord(l0) - map->xcoord(l1));
		int dy = abs(map->ycoord(l0) - map->ycoord(l1));
		int diag = dx < dy ? dx : dy;
		int straight = dx < dy ? dy : dx;
		return (straight - diag) + sqrtf(2.0) * diag;
	}

	unsigned int octilecells(unsigned int l0, unsigned int l1) const {
		int dx = abs(map->xcoord(l0) - map->xcoord(l1));
		int dy = abs(map->ycoord(l0) - map->ycoord(l1));
		return dx < dy ? dy : dx;
	}
};

#endif	// _GRIDPATH_HPP_
