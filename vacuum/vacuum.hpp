// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#pragma once

#include "../utils/utils.hpp"
#include "../gridnav/gridmap.hpp"
#include <memory>
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

		bool eq(const Vacuum*, const State &o) const {
			if (loc != o.loc || ndirt != o.ndirt)
				return false;

			for (unsigned int i = 0; i < dirt->size(); i++) {
				if (dirt->at(i) != o.dirt->at(i))
					return false;
			}

			return true;
		}

		unsigned long hash(const Vacuum*) const {
			return loc*dirt->size() + ndirt;
		}

		int loc, energy, ndirt;
		std::shared_ptr<std::vector<bool> > dirt;
	};

	typedef State PackedState;

	State initialstate(void) const;

	Cost h(const State &s) const {
		unsigned int i;
		for (i = 0; i < s.dirt->size() && !s.dirt->at(i); i++)
			;

		int minx = dirtLocs[i].first;
		int maxx = minx;
		int miny = dirtLocs[i].second;
		int maxy = miny;

		for (i++; i < s.dirt->size(); i++) {
			if (!s.dirt->at(i))
				continue;
			int x = dirtLocs[i].first, y = dirtLocs[i].second;
			if (x < minx)
				minx = x;
			if (x > maxx)
				maxx = x;
			if (y < miny)
				miny = y;
			if (y > maxy)
				maxy = y;
		}

		return s.ndirt + (maxx-minx) + (maxy-miny);
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
			if (dirt >= 0 && s.dirt->at(dirt))
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
				assert ((unsigned int) dirt < d.ndirt());
				assert (state.dirt->at(dirt));

				state.dirt = std::make_shared<std::vector<bool>>(s.dirt->begin(), s.dirt->end());
				state.dirt->at(dirt) = false;
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
		for (unsigned int i = 0; i < s.dirt->size(); i++)
			fprintf(out, " %d", (int) s.dirt->at(i));
	}

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

	void printpath(FILE*, const std::vector<Oper>&) const;

private:

	unsigned int ndirt() const {
		return dirtLocs.size();
	}

	// reverseops computes the reverse operators
	void reverseops();

	// rev holds the index of the reverse of each operator
	int rev[8];

	GridMap *map;
	int x0, y0;

	std::vector<int> dirt;	// Indexed by map location, gives dirt ID.
	std::vector<std::pair<int, int> > dirtLocs;
	std::vector<bool> chargers;
};
