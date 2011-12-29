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

	typedef float Cost;
	static const float InfCost = -1.0;

	typedef int Oper;	// Index into the ops arrays.
	enum { Nop = -1 };

	GridNav(GridMap*, unsigned int, unsigned int,
		unsigned int, unsigned int);

	class State {
		friend class GridNav;
		unsigned int loc;
		int nops;
		Oper ops[8];

	public:

		State &operator=(const State &o) {
 			loc = o.loc;
			nops = -1;
 			return *this;
		}

		State (const State &o) : loc(o.loc), nops(-1) { }

		State(void) :nops(-1) { /* memset(ops, 0, sizeof(ops)); */ }
	};

	struct PackedState {
		unsigned int loc;
		unsigned long hash(void) { return loc; }

		bool eq(const PackedState &other) const {
			return other.loc == loc;
		}
	};

	struct Undo {
		Undo(State &, Oper) { }
	};

	State initialstate(void);

	Cost h(State &s) {
		return octiledist(s.loc, finish);
	}

	Cost d(State &s) {
		return octilecells(s.loc, finish);
	}

	bool isgoal(State &s) {
		return s.loc == finish;
	}

	unsigned int nops(State &s) {
		if (s.nops < 0)
			s.nops = map->ops(s.loc, s.ops);
		return s.nops;
	}

	Oper nthop(State &s, unsigned int n) {
		return s.ops[n];
	}

	Oper revop(State &s, Oper op) {
		return s.loc;
	}

	void undo(State &s, Undo &u) { }

	State &apply(State &buf, State &s, Cost &c, Oper op) {
		c = sqrtf(2.0);
		if (map->x(s.loc) == map->x(op) || map->y(s.loc) == map->y(op))
			c = 1.0;
		buf.loc = op;
		return buf;
	}

	void pack(PackedState &dst, State &src) {
		dst.loc = src.loc;
	}

	State &unpack(State &buf, PackedState &pkd) {
		buf.loc = pkd.loc;
		return buf;
	}

	void dumpstate(FILE *out, State &s) {
		fprintf(out, "%u, %u\n", map->x(s.loc), map->y(s.loc));
	}

	unsigned int width(void) { return map->width(); }

	unsigned int height(void) { return map->height(); }

private:

	float octiledist(unsigned int l0, unsigned int l1) {
		unsigned int dx = abs(map->x(l0) - map->x(l1));
		unsigned int dy = abs(map->y(l0) - map->y(l1));
		unsigned int diag = dx < dy ? dx : dy;
		unsigned int straight = dx < dy ? dy : dx;
		return (straight - diag) + sqrtf(2.0) * diag;
	}

	unsigned int octilecells(unsigned int l0, unsigned int l1) {
		unsigned int dx = abs(map->x(l0) - map->x(l1));
		unsigned int dy = abs(map->y(l0) - map->y(l1));
		return dx < dy ? dy : dx;
	}

	unsigned int start, finish;
	GridMap *map;
};

#endif	// _GRIDPATH_HPP_
