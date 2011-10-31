#include <cstdio>
#include <cstdlib>
#include <cassert>

#if NCAKES >= 128
#error Too many pancakes for char typed cost.
#endif

extern "C" unsigned long hashbytes(unsigned char[], unsigned int);

class Pancake {
public:
	enum { UnitCost = true };
	enum { Ncakes = NCAKES };

	typedef int Cake;
	typedef int Oper;
	enum { Nop = -1 };
	typedef char Cost;
	enum { InfCost = -1 };

	class State {
	public:
		unsigned long hash(void) {
			return hashbytes((unsigned char *) cakes,
					Ncakes * sizeof(Cake));
		}

		bool eq(State &other) const {
			for (unsigned int i = 0; i < Ncakes; i++) {
				if (cakes[i] != other.cakes[i])
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

	class Undo {
	public:
		Undo(State &s, Oper o) : op(o), h(s.h) { }
	private:
		friend class Pancake;
		Oper op;
		Cost h;
	};

	typedef State PackedState;

	Pancake(FILE*);

	State initialstate(void);

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

	Oper revop(State &s, Oper op) {
		return op;
	}

	Cost opcost(State &s, Oper op) {
		return 1;
	}

	void undo(State &s, Undo &u) {
		s.h = u.h;
		s.flip(u.op);
	}

	State &apply(State &buf, State &s, Oper op) {
		bool wasgap = gap(s.cakes, op);
		s.flip(op);

		bool hasgap = gap(s.cakes, op);
		if (wasgap && !hasgap)
			s.h--;
		if (!wasgap && hasgap)
			s.h++;

		return s;
	}

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