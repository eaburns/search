// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>

#if NCAKES >= 128
#error Too many pancakes for char typed cost.
#endif

extern "C" unsigned long hashbytes(unsigned char[], unsigned int);

class Pancake {
public:
	enum { Ncakes = NCAKES };

	typedef int Cake;
	typedef int Oper;
	enum { Nop = -1 };
	typedef int Cost;

	struct State {
		bool eq(const Pancake*, const State &o) const {
			for (unsigned int i = 0; i < Ncakes; i++) {
				if (cakes[i] != o.cakes[i])
					return false;
			}
			return true;
		}

		unsigned long hash(const Pancake*) const {
			return hashbytes((unsigned char *) cakes,
						Ncakes * sizeof(Cake));
		}

	private:
		friend class Pancake;
		friend class Undo;
	
		void flip(Oper op) {
			assert (op > 0);
			assert (op < Ncakes);
	
			for (int n = 0; n <= op / 2; n++) {
				Cake tmp = cakes[n];
				cakes[n] = cakes[op - n];
				cakes[op - n] = tmp;
			}
		}

		Cake cakes[Ncakes];
		Cost h;
	};

	typedef State PackedState;

	Pancake(FILE*);

	State initialstate();

	Cost h(const State &s) const {
		return s.h;
	}

	Cost d(const State &s) const {
		return s.h;
	}

	bool isgoal(const State &s) const {
		return s.h == 0;
	}

	struct Operators {
		Operators(const Pancake&, const State&) { }

		unsigned int size() const {
			return Ncakes -1 ;
		}

		Oper operator[](unsigned int i) const {
			return i + 1;
		}
	};

	struct Edge {
		Cost cost;
		Oper revop;
		Cost revcost;
		State &state;

		Edge(Pancake &d, State &s, Oper op) :
				cost(1), revop(op), revcost(1), state(s), oldh(s.h) {
			bool wasgap = gap(state.cakes, op);
			state.flip(op);

			bool hasgap = gap(state.cakes, op);
			if (wasgap && !hasgap)
				state.h--;
			if (!wasgap && hasgap)
				state.h++;
		}

		~Edge() {
			state.h = oldh;
			state.flip(revop);
		}

	private:
		Cost oldh;
	};

	void pack(PackedState &dst, const State &s) const {
		dst = s;
	}

	State &unpack(const State &dst, PackedState &pkd) const {
		return pkd;
	}

	void dumpstate(FILE *out, State &s) const {
		for (unsigned int i = 0; i < Ncakes; i++) {
			fprintf(out, "%u", s.cakes[i]);
			if (i < Ncakes - 1)
				fputc(' ', out);
		}
		fputc('\n', out);
	}

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

private:

	static Cost ngaps(Cake cakes[]) {
		Cost gaps = 0;

		for (unsigned int i = 0; i < Ncakes; i++) {
			if (gap(cakes, i)) 
				gaps++;
		}

		return gaps;
	}

	// Is there a gap between cakes n and n+1?
	static bool gap(Cake cakes[], unsigned int n) {
		assert(n < Ncakes);
		if (n == Ncakes-1)
			return cakes[Ncakes-1] != Ncakes-1;
		return abs(cakes[n] - cakes[n+1]) != 1;
	}

	Cake init[Ncakes];
};