#include "../incl/search.hpp"

template <class D>
class Idastar {

public:

	typedef typename D::State State;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	Result<D> search(D &d, State *s0) {
		bound = d.h(s0);
		for (int i = 0; /* forever */; i++) {
			minoob = D::InfCost;

			if (dfs(d, s0, D::Nop, 0, 0))
				break;

			printf("iter %d, bound=%g, expd=%ld, gend=%ld\n",
				i, (double) bound, res.expd, res.gend);

			bound = minoob;
		}
		return res;
	}

private:

	bool dfs(D &d, State *s, Oper pop, Cost g, int depth) {
		Cost f = g + d.h(s);

		if ((D::UnitCost || f <= bound) && d.isgoal(s)) {
			res.cost = g;
			res.path.push_back(*s);
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

			d.apply(s, op);
			bool goal = dfs(d, s, rev, g + c, depth+1);
			d.undo(s, u);

			if (goal) {
				res.path.push_back(*s);
				return true;
			}
		}

		return false;
	}
	
	Result<D> res;
	Cost bound;
	Cost minoob;
};