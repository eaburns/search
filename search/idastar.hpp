#include <cstdio>
#include "../search/search.hpp"

void dfrowhdr(FILE *, const char *name, int ncols, ...);
void dfrow(FILE *, const char *name, const char *colfmt, ...);

template <class D> class Idastar : public SearchAlgorithm<D> {

public:

	typedef typename D::State State;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	Idastar(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv) { }

	Result<D> &search(D &d, State &s0) {
		this->start();
		bound = d.h(s0);
		dfrowhdr(stdout, "iter", 4, "iter no", "iter bound",
			"iter expd", "iter gend");

		for (int i = 0; !SearchAlgorithm<D>::limit(); i++) {
			minoob = Cost(-1);

			if (dfs(d, s0, D::Nop, Cost(0)))
				break;

			dfrow(stdout, "iter", "dguu", (long) i, (double) bound,
				SearchAlgorithm<D>::res.expd, SearchAlgorithm<D>::res.gend); 

			bound = minoob;
		}

		this->finish();
		return SearchAlgorithm<D>::res;
	}

private:
	bool dfs(D &d, State &s, Oper pop, Cost g) {
		Cost f = g + d.h(s);

		if (f <= bound && d.isgoal(s)) {
			SearchAlgorithm<D>::res.cost = g;
			SearchAlgorithm<D>::res.path.push_back(s);
			return true;
		}

		if (f > bound) {
			if (minoob == Cost(-1) || f < minoob)
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
			bool goal = false;
			{	// Put the transition in a new scope so that
				// it is destructed before we test for a goal.
				// If a goal was found then we want the
				// transition reverted so that we may push
				// the parent state onto the path.
				typename D::Transition tr(d, s, op);
				goal = dfs(d, tr.state, tr.revop, g + tr.cost);
			}

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
