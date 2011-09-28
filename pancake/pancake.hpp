#include "../incl/utils.hpp"
#include <cstdio>
#include <cassert>

#define NCAKES 80

#if NCAKES >= 128
#error Too many pancakes for char typed cost.
#endif

class Pancake {

	enum { Ncakes = NCAKES };

	typedef unsigned int Cake;
	typedef int Oper;
	enum { Nop = -1 };
	typedef char Cost;
	enum { Inifinity = -1 };

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
		Undo(State &s, Oper op) : n(op) {
			if (n < Ncakes / 2)
				copy.h = s.h;
			else
				copy = s;
		}
	private:
		State copy;
		Oper n;
	};

	typedef State PackedState;

	Pancake(FILE*);

	State initstate(void);

	Cost h(State &s) {
		return s.h;
	}

	bool isgoal(State &s) {
		assert("unimplemented" == 0);
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
		assert("unimplemented" == 0);
	}

	State &apply(State &buf, State &s, Oper newb) {
		assert("unimplemented" == 0);
	}

	void pack(PackedState &dst, State &s) {
		dst = s;
	}

	void unpack(State &dst, PackedState &pkd) {
		dst = pkd;
	}

	void dumpstate(FILE *out, State &s) {
		for (unsigned int i = 0; i < Ncakes; i++) {
			fprintf(out, "%u", s.cakes[i]);
			if (i < Ncakes - 1)
				fputc(' ', out);
		}
		fputc('\n', out);
	}

};