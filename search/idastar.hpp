#include <cstdio>
#include "../search/search.hpp"

void dfrowhdr(FILE *, const char *name, int ncols, ...);
void dfrow(FILE *, const char *name, const char *colfmt, ...);

template <class D> class Idastar : public Search<D> {

public:

	typedef typename D::State State;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	Result<D> &search(D &d, State &s0) {
		bound = d.h(s0);
		dfrowhdr(stdout, "iter", 4, "iter no", "iter bound",
			"iter expd", "iter gend");

		for (int i = 0; !Search<D>::limit(); i++) {
			minoob = D::InfCost;

			if (dfs(d, s0, D::Nop, 0))
				break;

			d.iterdone();

			dfrow(stdout, "iter", "dguu", (long) i, (double) bound,
				Search<D>::res.expd, Search<D>::res.gend); 

			bound = minoob;
		}

		Search<D>::res.finish();
		return Search<D>::res;
	}

	Idastar(int argc, char *argv[]) : Search<D>(argc, argv) { }

private:
	bool dfs(D &d, State &s, Oper pop, Cost g) {
		Cost f = g + d.h(s);

		if ((D::UnitCost || f <= bound) && d.isgoal(s)) {
			Search<D>::res.cost = g;
			Search<D>::res.path.push_back(s);
			return true;
		}

		if (f > bound) {
			if (minoob == D::InfCost || f < minoob)
				minoob = f;
			return false;
		}

		Search<D>::res.expd++;

		for (unsigned int n = 0; n < d.nops(s); n++) {
			if (Search<D>::limit())
				return false;
			Oper op = d.nthop(s, n);
			if (op == pop)
				continue;

			Search<D>::res.gend++;

			Undo u(s, op);
			Oper rev = d.revop(s, op);
			Cost c = d.opcost(s, op);

			State buf, &kid = d.apply(buf, s, op);
			bool goal = dfs(d, kid, rev, g + c);
			d.undo(s, u);

			if (goal) {
				Search<D>::res.path.push_back(s);
				return true;
			}
		}

		return false;
	}
	
	Cost bound;
	Cost minoob;
};
