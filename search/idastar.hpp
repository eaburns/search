#include <cstdio>
#include "../search/search.hpp"

void dfrowhdr(FILE *, const char *name, int ncols, ...);
void dfrow(FILE *, const char *name, const char *colfmt, ...);

template <class D> class Idastar : public SearchAlgorithm<D> {

public:

	typedef typename D::State State;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	Idastar(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv) { }

	Result<D> &search(D &d, State &s0) {
		SearchAlgorithm<D>::res.start();
		bound = d.h(s0);
		dfrowhdr(stdout, "iter", 4, "iter no", "iter bound",
			"iter expd", "iter gend");

		for (int i = 0; !SearchAlgorithm<D>::limit(); i++) {
			minoob = D::InfCost;

			if (dfs(d, s0, D::Nop, 0))
				break;

			dfrow(stdout, "iter", "dguu", (long) i, (double) bound,
				SearchAlgorithm<D>::res.expd, SearchAlgorithm<D>::res.gend); 

			bound = minoob;
		}

		SearchAlgorithm<D>::res.finish();
		return SearchAlgorithm<D>::res;
	}

private:
	bool dfs(D &d, State &s, Oper pop, Cost g) {
		Cost f = g + d.h(s);

		if ((D::UnitCost || f <= bound) && d.isgoal(s)) {
			SearchAlgorithm<D>::res.cost = g;
			SearchAlgorithm<D>::res.path.push_back(s);
			return true;
		}

		if (f > bound) {
			if (minoob == D::InfCost || f < minoob)
				minoob = f;
			return false;
		}

		SearchAlgorithm<D>::res.expd++;

		for (unsigned int n = 0; n < d.nops(s); n++) {
			if (SearchAlgorithm<D>::limit())
				return false;
			Oper op = d.nthop(s, n);
			if (op == pop)
				continue;

			SearchAlgorithm<D>::res.gend++;

			Undo u(s, op);
			Oper rev = d.revop(s, op);
			Cost c;
			State buf, &kid = d.apply(buf, s, c, op);
			bool goal = dfs(d, kid, rev, g + c);
			d.undo(s, u);

			if (goal) {
				SearchAlgorithm<D>::res.path.push_back(s);
				SearchAlgorithm<D>::res.ops.push_back(op);
				return true;
			}
		}

		return false;
	}
	
	Cost bound;
	Cost minoob;
};
