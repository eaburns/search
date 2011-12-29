#include "../search/search.hpp"
#include "../utils/utils.hpp"
#include <algorithm>

template <class D> struct Rtastar : public SearchAlgorithm<D> {
	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node {
		ClosedEntry<Node, D> closedent;
		PackedState packed;
		Cost h;

		static PackedState &key(Node *n) { return n->packed; }

		static bool eq(PackedState &a, PackedState &b) { return a.eq(b); }

		static unsigned long hash(PackedState &k) { return k.hash(); }

		static ClosedEntry<Node, D> &closedentry(Node *n) { return n->closedent; }
	};

	Rtastar(int argc, const char *argv[]) :
			SearchAlgorithm<D>(argc, argv), nlook(0),
			seen(30000001) {
		for (int i = 1; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-n") == 0)
				nlook = strtoul(argv[++i], NULL, 10);
		}
		nodes = new boost::object_pool<Node>();
	}

	~Rtastar(void) { delete nodes; }

	struct Current {
		Current(const State &s, Oper o, Oper p, Cost e, Cost _f) :
			state(s), op(o), pop(p), edgecost(e), f(_f) { }

		State state;
		Oper op, pop;
		Cost edgecost;
		Cost f;
	};

	Result<D> &search(D &d, typename D::State &s0) {
		SearchAlgorithm<D>::res.start();
		seen.init(d);
		Current cur(s0, D::Nop, D::Nop, 0, d.h(s0));
		Cost *curh = storenode(d, cur.state, cur.f);
		SearchAlgorithm<D>::res.cost = 0;
		SearchAlgorithm<D>::res.path.push_back(cur.state);

		while (!d.isgoal(cur.state) && !SearchAlgorithm<D>::limit()) {
			std::vector<Current> bests = bestkids(d, cur, *curh);
			if (bests.size() == 0)
				break;	// deadend;
			cur = bests.at(randgen.integer(0, bests.size()-1));
			SearchAlgorithm<D>::res.cost += cur.edgecost;
			SearchAlgorithm<D>::res.ops.push_back(cur.op);
			SearchAlgorithm<D>::res.path.push_back(cur.state);
			curh = storenode(d, cur.state, cur.f);
		}

		SearchAlgorithm<D>::res.finish();

		// Reverse the path and operators since all other
		// search algorithms build them in reverse.
		std::reverse(SearchAlgorithm<D>::res.ops.begin(),
			SearchAlgorithm<D>::res.ops.end());
		std::reverse(SearchAlgorithm<D>::res.path.begin(),
			SearchAlgorithm<D>::res.path.end());
		
		return SearchAlgorithm<D>::res;
	}

	virtual void reset(void) { seen.clear(); }

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		// We output the 'closed ' prefix here so that they key
		// matches the closed keys for other search algorithms.
		// This is desriable because the seen list performs a
		// similar functionality as a closed list, and we will
		// probably want to compare with closed list stats.
		seen.prstats(stdout, "closed ");
		dfpair(stdout, "lookahead depth", "%u", nlook);
		dfpair(stdout, "node size", "%u", sizeof(Node));
	}

private:

	// bestkids returns a vector of all of the successors that have the
	// best f value.
	std::vector<Current> bestkids(D &d, Current &cur, Cost &sndf) {
		sndf = D::InfCost;
		std::vector<Current> bests;

		SearchAlgorithm<D>::res.expd++;
		for (unsigned int n = 0; n < d.nops(cur.state); n++) {
			Oper op = d.nthop(cur.state, n);
			if (op == cur.pop)
				continue;

			SearchAlgorithm<D>::res.gend++;
			Undo u(cur.state, op);
			Cost cost;
			Oper rev = d.revop(cur.state, op);
			State buf, &kid = d.apply(buf, cur.state, cost, op);
			Cost h = heuristic(d, kid, rev);
			Cost f = h == D::InfCost ? h : h + cost;

			if (bests.empty() || better(f, bests[0].f)) {
				if (!bests.empty())
					sndf = bests[0].f;
				bests.clear();
				bests.push_back(Current(kid, op, rev, cost, f));
			} else if (bests[0].f == f) {
				assert (!bests.empty());
				bests.push_back(Current(kid, op, rev, cost, f));
			} else if (better(f, sndf)) {
				sndf = f;
			}

			d.undo(cur.state, u);
		}

		return bests;
	}

	// value returns the f value for the given state
	// either from the seen table or by performing
	// lookahead search.
	Cost heuristic(D &d, State &cur, Oper pop) {
		Node *n = nodes->construct();
		d.pack(n->packed, cur);
		unsigned long hash = n->packed.hash();

		Node *dup = seen.find(n->packed, hash);
		if (dup) {
			nodes->destroy(n);
			return dup->h;
		}
		Cost alpha = D::InfCost;
		n->h = look(d, cur, alpha, pop, 0, nlook);
		seen.add(n, hash);
		return n->h;
	}

	// look returns the lookahead cost via a depth-limited,
	// alpha pruned, depth-first search.
	Cost look(D &d, State &cur, Cost &alpha, Oper pop, Cost g, unsigned int left) {
		Cost f = d.h(cur) + g;
		if (left == 0 || !better(f, alpha) || d.isgoal(cur)) {
			if (better(f, alpha))
				alpha = f;
			return f;
		}

		SearchAlgorithm<D>::res.expd++;
		Cost bestf = D::InfCost;
		for (unsigned int n = 0; n < d.nops(cur); n++) {
			Oper op = d.nthop(cur, n);
			if (op == pop)
				continue;

			SearchAlgorithm<D>::res.gend++;
			Undo u(cur, op);
			Oper rev = d.revop(cur, op);
			Cost edgecost;
			State buf, &kid = d.apply(buf, cur, edgecost, op);
			f = look(d, kid, alpha, rev, g + edgecost, left-1);
			d.undo(cur, u);
			if (better(f, bestf))
				bestf = f;
		}

		return bestf;	
	}

	// storenode marks the node as seen if it is not already
	// in the seen list and returns a pointer to the stored
	// heuristic value.  This pointer can be used to update
	// the value of this node without looking it up again.
	Cost *storenode(D &d, State &cur, Cost h) {
		Node *n = nodes->construct();
		d.pack(n->packed, cur);
		unsigned long hash = n->packed.hash();

		Node *dup = seen.find(n->packed, hash);
		if (dup) {
			nodes->destroy(n);
			dup->h = h;
			return &dup->h;
		}

		n->h = h;
		seen.add(n, hash);
		return &n->h;
	}

	// better returns true if a is better than b.
	static bool better(Cost a, Cost b) {
		return b == D::InfCost || (a != D::InfCost && a < b);
	}

	unsigned int nlook;
	ClosedList<Node, Node, D> seen;
	boost::object_pool<Node> *nodes;
};