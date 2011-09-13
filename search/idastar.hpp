#include <cstdio>
#include "../incl/search.hpp"

void dfrowhdr(FILE *, const char *name, int ncols, ...);
void dfrow(FILE *, const char *name, const char *colfmt, ...);

template <class D, bool unitcost=false> class Idastar : public Search<D> {

public:

	typedef typename D::State State;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	Result<D> search(D &d, State &s0) {
		res = Result<D>(false);
		bound = d.h(s0);
		dfrowhdr(stdout, "iter", 4, "iter no", "iter bound",
			"iter expd", "iter gend");

		for (int i = 0; /* forever */; i++) {
			minoob = D::InfCost;

			if (dfs(d, s0, D::Nop, 0, 0))
				break;

			dfrow(stdout, "iter", "dguu", (long) i, (double) bound,
				res.expd, res.gend); 

			bound = minoob;
		}

		res.finish();
		return res;
	}

private:
	bool dfs(D &d, State &s, Oper pop, Cost g, int depth) {
		Cost f = g + d.h(s);

		if ((unitcost || f <= bound) && d.isgoal(s)) {
			res.cost = g;
			res.path.push_back(s);
			return true;
		}

		if (f > bound) {
			if (minoob == D::InfCost || f < minoob)
				minoob = f;
			return false;
		}

		res.expd++;

		for (unsigned int n = 0; n < d.nops(s); n++) {
			Oper op = d.nthop(s, n);
			if (op == pop)
				continue;

			res.gend++;

			Undo u(s, op);
			Oper rev = d.revop(s, op);
			Cost c = d.opcost(s, op);

			State buf, &kid = d.apply(buf, s, op);
			bool goal = dfs(d, kid, rev, g + c, depth+1);
			d.undo(s, u);

			if (goal) {
				res.path.push_back(s);
				return true;
			}
		}

		return false;
	}
	
	Result<D> res;
	Cost bound;
	Cost minoob;
};
