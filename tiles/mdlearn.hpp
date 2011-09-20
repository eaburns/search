#include "mdist.hpp"

class TilesMDLearn : public TilesMdist {
public:

	TilesMDLearn(FILE*);

	Oper nthop(State &s, unsigned int n) {
		return ops[s.b].mvs[n].b;
	}

	State &apply(State &buf, State &s, Oper newb) {
		Opinfo *info = ops[s.b].dests[newb];
		Cost oldh = s.h;
		State &res = TilesMdist::apply(buf, s, newb);
		Cost newh = res.h;
		info->nused++;
		if (newh < oldh)
			info->ndec++;
		return res;
	}

	void checkpoint(void) {
		sortops();
		initdests();
		resetprobs();
	}

private:
	friend int cmpmv(const void*, const void*);

	void initops(void);
	void sortops(void);
	void initdests(void);
	void resetprobs(void);

	struct Opinfo {
		Pos b;
		unsigned int nused;
		unsigned int ndec;	// Number of times h decreased.
	};
	struct Opvec {
		unsigned int n;
		Opinfo mvs[4];
		Opinfo *dests[Ntiles];
	};
	Opvec ops[Ntiles];
};