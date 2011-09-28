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
					return true;
			}
			return true;
		}
	private:
		friend class Pancake;
		Cake cakes[Ncakes];
		Cost h;
	};

	class Undo {
	public:
		Undo(State &s, Oper o) : op(o) { }
	private:
		friend class Pancake;
		Oper op;
	};

	typedef State PackedState;

	Pancake(FILE*);

	State initstate(void);

	Cost h(State &s) {
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
		if (inplace(u.op)) {
			if (!gap(s, u.op))
				s.h++;
			flip(s, u.op);
		}
	}

	State &apply(State &buf, State &s, Oper op) {
		State &res = s;

		if (inplace(op))
			flip(s, op);
		else
			res = flipinto(buf, s, op);

		if (!gap(res, op-1))
			res.h--;

		return res;
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

	void checkpoint(void) { }

private:

	bool inplace(Oper op) {
		return op < Ncakes / 2;
	}

	State &flipinto(State &dst, State &s, Oper op) {
		assert (op > 1);
		assert (op <= Ncakes);

		for (int m = 0, n = op - 1; n >= 0; n--, m++)
			dst.cakes[m] = s.cakes[n];
		for (int n = op; n < Ncakes; n++)
			dst.cakes[n] = s.cakes[n];

		return dst;
	}

	void flip(State &s, Oper op) {
		assert (op > 1);
		assert (op <= Ncakes);

		for (int n = 0; n < op; n++) {
			Cake tmp = s.cakes[n];
			s.cakes[n] = s.cakes[op - 1 - n];
			s.cakes[op - 1 - n] = tmp;
		}
	}

	unsigned int ngaps(State &s) {
		unsigned int gaps = 0;

		for (unsigned int i = 0; i < Ncakes - 1; i++) {
			if (gap(s, i)) 
				gaps++;
		}

		return gaps;
	}

	// Is there a gap between cakes n and n+1?
	bool gap(State &s, unsigned int n) {
		assert(n != 0);
		return abs(s.cakes[n] - s.cakes[n+1]) == 1;
	}

	Cake init[Ncakes];
};