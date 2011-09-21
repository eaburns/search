#include "mdist.hpp"
#include <vector>
#include <boost/array.hpp>

class TilesMDLearn : public TilesMdist {
public:

	TilesMDLearn(FILE*);

	class State : public TilesMdist::State {
		friend class TilesMDLearn;
		State(const TilesMdist::State &s) : TilesMdist::State(s), d(0) { }
	protected:
		unsigned int d;
	public:
		State() : d(0) {}
	};

	State initstate(void) {
		return State(TilesMdist::initstate());
	}

	Oper nthop(State &s, unsigned int n) {
		if (s.d >= ops.size())
			initops(s.d);
		return ops[s.d][s.b].mvs[n].b;
	}

	void undo(State &s, Undo &u) {
		TilesMdist::undo(s, u);
		s.d--;
	}

	State &apply(State &buf, State &s, Oper newb) {
		Opinfo *info = ops[s.d][s.b].dests[newb];
		Cost oldh = s.h;
		TilesMdist::apply(buf, s, newb);
		s.d++;
		if (s.h < oldh)
			info->ndec++;
		else
			info->nother++;
		return s;
	}

	void checkpoint(void) {
		sortops();
		resetprobs();
	}

private:
	friend int cmpmv(const void*, const void*);

	void initops(unsigned int);
	void initdests(unsigned int);
	void sortops(void);
	void resetprobs(void);

	struct Opinfo {
		Pos b;
		unsigned int nother;
		unsigned int ndec;	// Number of times h decreased.
		bool operator<(const Opinfo &o) const {
			return (double) ndec / (double) (nother + ndec)
				> (double) o.ndec / (double) (o.nother + o.ndec);
		}
	};
	struct Opvec {
		unsigned int n;
		boost::array<Opinfo,4> mvs;
		Opinfo *dests[Ntiles];
	};
	std::vector< boost::array<Opvec, Ntiles> > ops;
};
