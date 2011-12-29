#include "../search/search.hpp"
#include "../utils/utils.hpp"

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
		Cost *curh = see(d, cur.state, cur.f);
		SearchAlgorithm<D>::res.cost = 0;
		SearchAlgorithm<D>::res.path.push_back(cur.state);

		while (!d.isgoal(cur.state) && !SearchAlgorithm<D>::limit()) {
			Cost sndf;
			std::vector<Current> bests = bestkids(d, cur, sndf);
			if (bests.size() == 0)
				break;	// deadend;
			*curh = sndf;
			cur = bests.at(randgen.integer(0, bests.size()-1));
			SearchAlgorithm<D>::res.cost += cur.edgecost;
			SearchAlgorithm<D>::res.path.push_back(cur.state);
			SearchAlgorithm<D>::res.ops.push_back(cur.op);
			curh = see(d, cur.state, cur.f);
		}

		SearchAlgorithm<D>::res.finish();
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
		n->h = lookahead(d, cur, pop);
		seen.add(n, hash);
		return n->h;
	}

	// lookahead performs minimin lookahead beneath
	// the given node.
	Cost lookahead(D &d, State &cur, Oper pop) {
		Cost alpha = D::InfCost;
		return look(d, cur, alpha, pop, 0, nlook);
	}

	// look returns the lookahead cost.  This performs a
	// minimin depth-first search for no more than 'left'
	// nodes.
	Cost look(D &d, State &cur, Cost &alpha, Oper pop, Cost cost, unsigned int left) {
		Cost f = d.h(cur) + cost;
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
			f = look(d, kid, alpha, rev, cost + edgecost, left-1);
			d.undo(cur, u);
			if (better(f, bestf))
				bestf = f;
		}

		return bestf;	
	}

	// see marks the node as seen if it is not already in
	// the seen list.  The return value is a pointer to the
	// stored heuristic value for this node so that it
	// may be update at a later time without requiring
	// another hash table lookup. 
	Cost *see(D &d, State &cur, Cost h) {
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