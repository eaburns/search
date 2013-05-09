#pragma once

#include "gridmap.hpp"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>

// controlvec converts a string of controls back into a vector.
std::vector<unsigned int> controlvec(const std::string&);

std::string controlstr(const std::vector<unsigned int>&);

struct Traffic {

	struct Cost {
		Cost() { }

		explicit Cost(int u, int s = 0) : units(u), sqrts(s) { compute(); }

		Cost &operator=(const Cost &o) {
			units = o.units;
			sqrts = o.sqrts;
			val = o.val;
			return *this;
		}

		Cost &operator+=(const Cost &o) {
			units += o.units;
			sqrts += o.sqrts;
			compute();
			return *this;
		}

		Cost &operator-=(const Cost &o) {
			units -= o.units;
			sqrts -= o.sqrts;
			compute();
			return *this;
		}

		operator double() const { return val; }

		Cost operator+(const Cost &o) const {
			return Cost(units + o.units, sqrts + o.sqrts);
		}

		Cost operator-(const Cost &o) const {
			return Cost(units - o.units, sqrts - o.sqrts);
		}

		bool operator<(const Cost &o) const {
			return val < o.val;
		}

		bool operator>(const Cost &o) const {
			return val > o.val;
		}

		bool operator==(const Cost &o) const {
			return units == o.units && sqrts == o.sqrts;
		}

		bool operator!=(const Cost &o) const {
			return !(*this == o);
		}

		bool operator>=(const Cost &o) const {
			return !(*this < o);
		}

		bool operator<=(const Cost &o) const {
			return !(*this > o);
		}

	private:

		void compute() {
			val = units + sqrts * sqrt(2);
		}

		int units, sqrts;
		double val;
	};

	typedef int Oper;	// Index into the ops arrays.
	enum {
		Nop = -1,
		Ident = 8,	// 0-7 movement ops (max), so 8 is safe for ident operation.
	};

	Traffic(GridMap*, unsigned int, unsigned int,
		unsigned int, unsigned int);

	struct State {
		State &operator=(const State &o) {
 			loc = o.loc;
			t = o.t;
			h = o.h;
			d = o.d;
 			return *this;
		}

		State() : d(-1) { }

		State (const State &o) : loc(o.loc), t(o.t), h(o.h), d(o.d) { }

		State(unsigned int l, unsigned int t) : loc(l), t(t), d(-1) { }

	private:
		friend struct Traffic;
		unsigned int loc, t;
		Cost h;
		int d;
	};

	struct PackedState {
		unsigned int loc, t;

		unsigned long hash(const Traffic*) const {
			return loc;
		}

		bool eq(const Traffic*, const PackedState &o) const {
			return o.loc == loc && o.t == t;
		}

	};

	State initialstate() const;

	Cost h(State &s) const {
		computeh(s);
		return s.h;
	}

	Cost d(State &s) const {
		computeh(s);
		return Cost(s.d);
	}

 	// Ident returns the (identity action, cost) pair, or (Nop, -1) if there is no identity action in this state.
	std::pair<Oper, Cost> ident(const State &s) const {
		return std::make_pair(Ident, Cost(1));
	}

	void computeh(State &s) const {
		if (s.d >= 0)
			return;
		s.d = manhattan(s.loc, finish);
		s.h = Cost(s.d);
	}

	bool isgoal(State &s) const {
		return s.loc == finish;
	}

	struct Operators {
		Operators(const Traffic &d, const State &s) : n(0) {

			for (unsigned int i = 0; i < d.map->nmvs; i++) {

				if (d.map->ok(s.loc, s.t+1, d.map->mvs[i])) {
					ops[n++] = i;
				}
			}
		}

		unsigned int size() const {
			return n;
		}

		Oper operator[](unsigned int i) const {
			return ops[i];
		}

	private:
		unsigned int n;
		Oper ops[8];
	};

	struct Edge {
		Cost cost;
		Oper revop;
		Cost revcost;
		State state;

		Edge(const Traffic &d, State &s, Oper op) : revop(Nop), revcost(-1), state(s) {
			assert (s.loc < d.map->sz);


			int newLoc = s.loc + d.map->mvs[op].delta;
			int newT = s.t + 1;
			bool teleport = false;\

			for(auto obs : d.map->obstacles) {
				std::pair<unsigned int, unsigned int> o = obs.positionAt(d.map->w,d.map->h,newT);
				if(newLoc  == d.map->index(o.first, o.second)) {
					teleport = true;
					break;
				}
			}

			if(teleport)
				state = d.initialstate();
			else
				state = State(s.loc + d.map->mvs[op].delta, s.t+1);

			assert (state.loc < d.map->sz);
			if (op == Ident || d.map->mvs[op].cost == 1.0)
				cost = Cost(1, 0);
			else
				cost = Cost(0, 1);
		}
	};

	void pack(PackedState &dst, State &src) const {
		dst.loc = src.loc;
		dst.t = src.t;
	}

	State &unpack(State &buf, PackedState &pkd) const {
		buf.loc = pkd.loc;
		buf.t = pkd.t;
		return buf;
	}

	void dumpstate(FILE *out, State &s) const {
		std::pair<int,int> coord = map->coord(s.loc);
		fprintf(out, "%u, %u @ %u\n", coord.first, coord.second, s.t);
	}

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&) const;

	unsigned int start, finish;
	GridMap *map;

	// rev holds the index of the reverse of each operator
	int rev[8];

private:

	// manhattan returns the Manhattan distance.
	int manhattan(int l0, int l1) const {
		auto c0 = map->coord(l0), c1 = map->coord(l1);
		unsigned int d = abs(c0.first - c1.first) + abs(c0.second - c1.second);
		return d;
	}

	// octiledist returns an admissible heuristic estimate
	// and cost estimate for eight-way grids.
	std::pair<Cost, int> octiledist(int l0, int l1) const {
		auto c0 = map->coord(l0), c1 = map->coord(l1);
		int dx = abs(c0.first - c1.first);
		int dy = abs(c0.second - c1.second);
		int shorter = dx < dy ? dx : dy;
		int longer = dx < dy ? dy : dx;
		return std::make_pair(Cost(longer - shorter, shorter), longer);
	}

	// reverseops computes the reverse operators
	void reverseops() {}
};
