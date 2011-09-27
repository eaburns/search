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
	void initops(unsigned int);

	void sortops(void) {
		for (unsigned int d = 0; d < ops.size(); d++) {
			for (unsigned int i = 0; i < Ntiles; i++)
				ops[d][i].sort();
		}
	}
	
	void resetprobs(void) {
		for (unsigned int d = 0; d < ops.size(); d++) {
		for (unsigned int i = 0; i < Ntiles; i++)
			ops[d][i].resetcounts();
		}
	}
	 
	struct Opinfo {
		Pos b;
		unsigned int nother;
		unsigned int ndec;	// Number of times h decreased.

		void resetcounts(void) {
			nother = ndec = 0;
		}

		double prob(void) const {
			return (double) ndec / (double) (ndec  + nother);
		}

		bool operator<(const Opinfo &other) const {
			return prob() < other.prob();
		}
	};

	struct Opvec {
		unsigned int n;
		boost::array<Opinfo, 4> mvs;
		Opinfo *dests[Ntiles];

		void init(Pos b);

		void initdests(void) {
			for (unsigned int i = 0; i < n; i++)
				dests[mvs[i].b] = &mvs[i];
		}

		void sort(void) {
			std::sort(mvs.begin(), mvs.begin()+n);
			initdests();
		}

		void resetcounts(void) {
			for (unsigned int i = 0; i < 4; i++)
				mvs[i].resetcounts();
		}
	};
	std::vector< boost::array<Opvec, Ntiles> > ops;
};
