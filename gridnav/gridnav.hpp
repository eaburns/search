#pragma once

#include "gridmap.hpp"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>

struct GridNav {

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
	enum { Nop = -1 };

	GridNav(GridMap*, unsigned int, unsigned int,
		unsigned int, unsigned int);

	struct State {
		State &operator=(const State &o) {
 			loc = o.loc;
			h = o.h;
			d = o.d;
 			return *this;
		}

		State() : d(-1) { }

		State (const State &o) : loc(o.loc), h(o.h), d(o.d) { }

		State(unsigned int l) : loc(l), d(-1) { }

	private:
		friend struct GridNav;
		unsigned int loc;
		Cost h;
		int d;
	};

	struct PackedState {
		unsigned int loc;

		bool operator==(const PackedState &o) const {
			return o.loc == loc;
		}
	};

	State initialstate() const;

	unsigned long hash(PackedState &p) {
		return p.loc;
	}

	Cost h(State &s) const {
		computeh(s);
		return s.h;
	}

	Cost d(State &s) const {
		computeh(s);
		return Cost(s.d);
	}

	void computeh(State &s) const {
		if (s.d >= 0)
			return;

		if (map->nmvs > 4 && !map->lifecost) {
			auto hd = octiledist(s.loc, finish);
			s.h = hd.first;
			s.d = hd.second;
		} else if (map->nmvs > 4 && map->lifecost) {
			auto hd = life8cheap(s.loc, finish);
			s.h = hd.first;
			s.d = hd.second;
		} else if (map->nmvs <= 4 && map->lifecost) {
			auto hd = life4cheap(s.loc, finish);
			s.h = hd.first;
			s.d = hd.second;
		} else {
			s.d = manhattan(s.loc, finish);
			s.h = Cost(s.d);
		}
	}

	bool isgoal(State &s) const {
		return s.loc == finish;
	}

	struct Operators {
		Operators(const GridNav &d, const State &s) : n(0) {
			for (unsigned int i = 0; i < d.map->nmvs; i++) {
				if (d.map->ok(s.loc, d.map->mvs[i]))
					ops[n++] = i;
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
		State state;

		Edge(const GridNav &d, State &s, Oper op) :
				revop(d.rev[op]),
				state(s.loc + d.map->mvs[op].delta) {
			assert (state.loc < d.map->sz);
			int mul = 1;
			if (d.map->lifecost) {
				// -2 because there is 1 layer of boarder
				// around each side of the grid, thus
				// height - y - 1 becomes height - y - 2.
				mul = (int) d.map->h - d.map->ycoord(s.loc)-2;
			}
			if (d.map->mvs[op].cost == 1.0)
				cost = Cost(mul, 0);
			else
				cost = Cost(0, mul);
		}
	};

	void pack(PackedState &dst, State &src) const {
		dst.loc = src.loc;
	}

	State &unpack(State &buf, PackedState &pkd) const {
		buf.loc = pkd.loc;
		return buf;
	}

	void dumpstate(FILE *out, State &s) const {
		std::pair<int,int> coord = map->coord(s.loc);
		fprintf(out, "%u, %u\n", coord.first, coord.second);
	}

	// pathcost returns the cost of the given path.
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

	// lifecost is the cost of moving out of cell y.
	int lifecost(int y) const {
		return map->h - y - 3;
	}

	// sumbetween returns the sum of n numbers
	// between a and b, inclusive.
	int sumbetween(int a, int b, int n) const {
		return (a + b)*n / 2;
	}

	// costfrom returns the life cost for moving between
	// y0 and y1.
	int costfrom(int y0, int y1) const {
		int last = y1;
		if (y0 > y1)
			last++;
		if (y0 < y1)
			last--;
		return sumbetween(lifecost(y0), lifecost(last), abs(y0 - y1));
	}

	// life4cheap returns an admissible heuristic cost
	// estimate and distance estimate for four-way
	// life-cost grids.
	std::pair<Cost, int> life4cheap(int l0, int l1) const {
		auto c0 = map->coord(l0), c1 = map->coord(l1);
		int x = c0.first - 1, gx = c1.first - 1;
		int y = c0.second - 1, gy = c1.second - 1;
		int h = map->h - 2;
		int dx = abs(gx - x);
		int overcost = lifecost(y > gy ? y : gy);
		int upover = dx*overcost + costfrom(y, gy);
		int upoverdown = costfrom(y, h) + costfrom(h, gy);

		if (upover < upoverdown)
			return std::make_pair(upover, dx+abs(gy - y));

		return std::make_pair(upoverdown, dx + h-1-y + h-1-gy);
	}

	// life8cheap returns an admissible heuristic cost
	// and distance estimate for eight-way life-cost grids.
	std::pair<Cost, int> life8cheap(int l0, int l1) const {
		auto c0 = map->coord(l0), c1 = map->coord(l1);
		int x = c0.first - 1, gx = c1.first - 1;
		int y = c0.second - 1, gy = c1.second - 1;
		int dx = abs(gx - x);
		int dy = abs(gy - y);

		if (dx <= dy)
			return std::make_pair(costfrom(y, gy), dy);

		int h = map->h - 2;
		int maxy = h - 1;
		int maxup = (maxy-y) < (maxy-gy) ? maxy-y : maxy - gy;
		int extra = dx - dy;
		int up = maxup < extra/2 ? maxup : extra/2;
		int highy = (y > gy ? y : gy) + up;
		int across = extra - 2*up;
		int c = costfrom(y, highy) + across*lifecost(highy) + costfrom(highy, gy);
		return std::make_pair(c, dx);
	}

	// reverseops computes the reverse operators
	void reverseops();
};
