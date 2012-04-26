#include <cstdio>
#include <cstdlib>
#include <cassert>

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
		bool operator==(const State &o) const {
			for (unsigned int i = 0; i < Ncakes; i++) {
				if (cakes[i] != o.cakes[i])
					return false;
			}
			return true;
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

	State initialstate(void);

	unsigned long hash(PackedState &p) {
		return hashbytes((unsigned char *) p.cakes,
					Ncakes * sizeof(Cake));
	}

	Cost h(State &s) {
		return s.h;
	}

	Cost d(State &s) {
		return s.h;
	}

	bool isgoal(State &s) {
		return s.h == 0;
	}

	unsigned int nops(State &s) {
		return Ncakes - 1;
	}

	Oper nthop(State &s, unsigned int n) {
		return n + 1;
	}

	struct Edge {
		Cost cost;
		Oper revop;
		State &state;

		Edge(Pancake &d, State &s, Oper op) :
				cost(1), revop(op), state(s), oldh(s.h) {
			bool wasgap = gap(state.cakes, op);
			state.flip(op);

			bool hasgap = gap(state.cakes, op);
			if (wasgap && !hasgap)
				state.h--;
			if (!wasgap && hasgap)
				state.h++;
		}

		~Edge(void) {
			state.h = oldh;
			state.flip(revop);
		}

	private:
		Cost oldh;
	};

	void pack(PackedState &dst, State &s) {
		dst = s;
	}

	State &unpack(State &dst, PackedState &pkd) {
		return pkd;
	}

	void dumpstate(FILE *out, State &s) {
		for (unsigned int i = 0; i < Ncakes; i++) {
			fprintf(out, "%u", s.cakes[i]);
			if (i < Ncakes - 1)
				fputc(' ', out);
		}
		fputc('\n', out);
	}

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