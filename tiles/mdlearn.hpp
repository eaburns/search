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
		return ops[s.d][s.b].sorted[n].b();
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
	};

	// Wrap the pointer in a class so that it has operator<
	// and can be sorted.
	struct OpinfoPtr {
		OpinfoPtr(void) : ptr(NULL) { }

		bool operator<(const OpinfoPtr &other) const {
			return ptr->prob() > other.ptr->prob();
		}

		Pos b(void) const { return ptr->b; }

		Opinfo *ptr;
	};

	struct Opvec {
		unsigned int n;
		Opinfo mvs[4];
		Opinfo *dests[Ntiles];
		boost::array<OpinfoPtr, 4> sorted;

		void init(Pos b);

		void sort(void) {
			std::sort(sorted.begin(), sorted.begin()+n);
		}

		void resetcounts(void) {
			for (unsigned int i = 0; i < 4; i++)
				mvs[i].resetcounts();
		}
	};
	std::vector<Opvec*> ops;
};
