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

	Cost h(State &s) const {
		if (map->nmvs > 4)
			return octiledist(s.loc, finish);
		return manhattan(s.loc, finish);
	}

	Cost d(State &s) const { return octilecells(s.loc, finish); }

	bool isgoal(State &s) const { return s.loc == finish; }

	unsigned int nops(State &s) const {
		if (s.nops > 0)
			return s.nops;

		s.nops = 0;
		for (unsigned int i = 0; i < map->nmvs; i++) {
			if (map->ok(s.loc, map->mvs[i]))
				s.ops[s.nops++] = i;
		}
		return s.nops;
	}

	Oper nthop(State &s, unsigned int n) const { return s.ops[n]; }

	struct Transition {
		Cost cost;
		Oper revop;
		State state;

		Transition(GridNav &d, State &s, Oper op) :
			cost(d.map->mvs[op].cost), revop(d.rev[op]),
			state(s.loc + d.map->mvs[op].delta) { }
	};

	void pack(PackedState &dst, State &src) const { dst.loc = src.loc; }

	State &unpack(State &buf, PackedState &pkd) const {
		buf.loc = pkd.loc;
		return buf;
	}

	void dumpstate(FILE *out, State &s) const {
		std::pair<int,int> coord = map->coord(s.loc);
		fprintf(out, "%u, %u\n", coord.first, coord.second);
	}

	int start, finish;
	GridMap *map;

	// rev holds the index of the reverse of each operator
	int rev[8];

private:

	float manhattan(int l0, int l1) const {
		std::pair<int,int> c0 = map->coord(l0), c1 = map->coord(l1);
		return abs(c0.first - c1.first) + abs(c0.second - c1.second);
	}

	float octiledist(int l0, int l1) const {
		std::pair<int,int> c0 = map->coord(l0), c1 = map->coord(l1);
		int dx = abs(c0.first - c1.first);
		int dy = abs(c0.second - c1.second);
		int diag = dx < dy ? dx : dy;
		int straight = dx < dy ? dy : dx;
		return (straight - diag) + sqrtf(2.0) * diag;
	}

	unsigned int octilecells(unsigned int l0, unsigned int l1) const {
		std::pair<int,int> c0 = map->coord(l0), c1 = map->coord(l1);
		int dx = abs(c0.first - c1.first);
		int dy = abs(c0.second - c1.second);
		return dx < dy ? dy : dx;
	}

	// reverseops computes the reverse operators
	void reverseops(void);
};

#endif	// _GRIDPATH_HPP_
