#pragma once

#include "../utils/utils.hpp"
#include "../gridnav/gridmap.hpp"
#include <cstdio>

class Vacuum {
public:
	typedef int Cost;

	typedef int Oper;	// directions 0, 1, 2, 3
	static const Oper Suck = 4;
	static const Oper Charge = 5;
	static const Oper Nop = -1;

	Vacuum(FILE*);

	class State {
	public:
		State() : loc(-1), energy(-1), ndirt(-1) {
		}

		bool operator==(const State &o) const {
			if (loc != o.loc || ndirt != o.ndirt)
				return false;

			for (unsigned int i = 0; i < dirt.size(); i++) {
				if (dirt[i] != o.dirt[i])
					return false;
			}

			return true;
		}

		int loc, energy, ndirt;
		std::vector<bool> dirt;
	};

	typedef State PackedState;

	unsigned long hash(const PackedState &s) const {
		return s.loc*ndirt + s.ndirt;
	}

	State initialstate(void) const;

	Cost h(const State &s) const {
		return s.ndirt;
	}

	Cost d(const State &s) const {
		return h(s);
	}

	bool isgoal(const State &s) const {
		return s.ndirt == 0;
	}

	struct Operators {
		Operators(const Vacuum &d, const State &s) : n(0) {
			if (s.energy == 0)
				return;

			int dirt = d.dirt[s.loc];
			if (dirt >= 0 && s.dirt[dirt])
				ops[n++] = Suck;

			for (unsigned int i = 0; i < d.map->nmvs; i++) {
				if (d.map->ok(s.loc, d.map->mvs[i]))
					ops[n++] = i;
			}

			if (d.chargers[s.loc])
				ops[n++] = Charge;
		}

		unsigned int size() const {
			return n;
		}

		Oper operator[] (unsigned int i) const {
			return ops[i];
		}

	private:
		unsigned int n;
		Oper ops[6];	// Up, Down, Left, Right, Suck, Charge
	};

	struct Edge {
		State state;
		Cost cost;
		Oper revop;
		Cost revcost;

		Edge(const Vacuum &d, const State &s, Oper op) : state(s), cost(1), revcost(1) {
			if (op == Suck) {
				int dirt = d.dirt[s.loc];

				assert (dirt >= 0);
				assert ((unsigned int) dirt < d.ndirt);
				assert (state.dirt[dirt]);

				state.dirt[dirt]  =false;
				state.ndirt--;

				revop = Nop;
				revcost = Cost(-1);

			} else if (op == Charge) {
				fatal("Charge operator!");

			} else {
				assert (op >= 0);
				assert (op <= 3);
				revop = d.rev[op];
				state.loc += d.map->mvs[op].delta;
			}
		}

		~Edge(void) { }
	};

	void pack(PackedState &dst, State &src) const {
		dst = src;
	}

	State &unpack(State &buf, PackedState &pkd) const {
		return pkd;
	}

	void dumpstate(FILE *out, const State &s) const {
		auto pt = map->coord(s.loc);
		fprintf(out, "(%d, %d), energy=%d, ndirt=%d", pt.first, pt.second, s.energy, s.ndirt);
		for (auto d : s.dirt)
			fprintf(out, " %d", d);
	}

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

private:

	// reverseops computes the reverse operators
	void reverseops();

	// rev holds the index of the reverse of each operator
	int rev[8];

	GridMap *map;
	int x0, y0;

	unsigned int ndirt;
	std::vector<int> dirt;
	std::vector<bool> chargers;
};