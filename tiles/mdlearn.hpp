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
			initops(s.d * 2);
		return ops[s.d][s.b].mvs[n].b;
	}

	void undo(State &s, Undo &u) {
		TilesMdist::undo(s, u);
		s.d--;
	}

	State &apply(State &buf, State &s, Oper newb) {
		assert (s.d < ops.size());
		Pos oldb = s.b;
		Opinfo *info = ops[s.d][oldb].dests[newb];
		Cost oldh = s.h;
		TilesMdist::apply(buf, s, newb);
		s.d++;
		Cost newh = s.h;
		info->nused++;
		if (newh < oldh)
			info->ndec++;
		return s;
	}

	void pack(PackedState &dst, State &s) {
		s.ts[s.b] = 0;
		dst.pack(s.ts);
	}

	State &unpack(State &buf, PackedState &pkd) {
		buf.b = pkd.unpack_md(md, buf.ts, &buf.h);
		return buf;
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
		unsigned int nused;
		unsigned int ndec;	// Number of times h decreased.
	};
	struct Opvec {
		unsigned int n;
		Opinfo mvs[4];
		Opinfo *dests[Ntiles];
	};
	std::vector< boost::array<Opvec, Ntiles> > ops;
};