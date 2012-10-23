// Best-first Utility-Guided Search Yes! From:
// "Best-first Utility-Guided Search," Wheeler Ruml and Minh B. Do,
// Proceedings of the Twentieth International Joint Conference on
// Artificial Intelligence (IJCAI-07), 2007

#include "../search/search.hpp"
#include "../structs/binheap.hpp"
#include "../utils/pool.hpp"

template <class D> struct Bugsy_slim : public SearchAlgorithm<D> {
	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node : SearchNode<D> {
		typename D::Cost h, d, f;
		double u, t;

		// The expansion count when this node was
		// generated.
		unsigned long expct;

		static bool pred(Node *a, Node *b) {
			if (a->u != b->u)
				return a->u > b->u;
			if (a->t != b->t)
				return a->t < b->t;
			if (a->f != b->f)
				return a->f < b->f;
			return a->g > b->g;
		}
	};

	enum {
		// Resort1 is the number of expansions to perform
		// before the 1st open list resort.
		Resort1 = 128,
	};

	Bugsy_slim(int argc, const char *argv[]) :
			SearchAlgorithm<D>(argc, argv), lastdelay(0), avgdelay(0),
			timeper(0.0), lasttime(0.0), lastexpd(0),
			nextresort(Resort1), nresort(0), closed(30000001) {
		wf = wt = -1;
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-wf") == 0)
				wf = strtod(argv[++i], NULL);
			else if (i < argc - 1 && strcmp(argv[i], "-wt") == 0)
				wt = strtod(argv[++i], NULL);
		}

		if (wf < 0)
			fatal("Must specify non-negative f-weight using -wf");
		if (wt < 0)
			fatal("Must specify non-negative t-weight using -wt");

		nodes = new Pool<Node>();
	}

	~Bugsy_slim() {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {
		this->start();
		lasttime = walltime();
		closed.init(d);
		Node *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		while (!open.empty() && !SearchAlgorithm<D>::limit()) {
			Node* n = *open.pop();
			State buf, &state = d.unpack(buf, n->packed);
			if (d.isgoal(state)) {
				SearchAlgorithm<D>::res.goal(d, n);
				break;
			}
			expand(d, n, state);
			updateopen();
		}

		this->finish();
	}

	virtual void reset() {
		SearchAlgorithm<D>::reset();
		open.clear();
		closed.clear();
		delete nodes;
		timeper = 0.0;
		lastexpd = 0;
		avgdelay = 0;
		lastdelay = 0;
		nresort = 0;
		nextresort = Resort1;
		nodes = new Pool<Node>();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", "binary heap");
		dfpair(stdout, "node size", "%u", sizeof(Node));
		dfpair(stdout, "wf", "%g", wf);
		dfpair(stdout, "wt", "%g", wt);
		dfpair(stdout, "final time per expand", "%g", timeper);
		dfpair(stdout, "number of resorts", "%lu", nresort);
		dfpair(stdout, "mean expansion delay", "%g", avgdelay);
	}

private:

	// Kidinfo holds information about a node used for
	// correcting the heuristic estimates.
	struct Kidinfo {
		Kidinfo() : f(-1), h(-1), d(-1) { }

		Kidinfo(Cost gval, Cost hval, Cost dval) : f(gval + hval), h(hval), d(dval) { }

		Cost f, h, d;
	};

	// expand expands the node, adding its children to the
	// open and closed lists as appropriate.
	void expand(D &d, Node *n, State &state) {
		this->res.expd++;

		unsigned long delay = this->res.expd - n->expct;
		avgdelay = avgdelay + (delay - avgdelay)/this->res.expd;

		Kidinfo bestinfo;
		typename D::Operators ops(d, state);
		for (unsigned int i = 0; i < ops.size(); i++) {
			if (ops[i] == n->pop)
				continue;

			this->res.gend++;

			Kidinfo kinfo = considerkid(d, n, state, ops[i]);
			if (bestinfo.f < Cost(0) || kinfo.f < bestinfo.f)
				bestinfo = kinfo;
		}

		if (bestinfo.f < Cost(0))
			return;
	}

	// considers adding the child to the open and closed lists.
	Kidinfo considerkid(D &d, Node *parent, State &state, Oper op) {
		Node *kid = nodes->construct();
		typename D::Edge e(d, state, op);
		kid->g = parent->g + e.cost;
		kid->d = d.d(e.state);
		kid->h = d.h(e.state);
		kid->f = kid->g + kid->h;
		kid->expct = this->res.expd;
		Kidinfo kinfo(kid->g, kid->h, kid->d);

		d.pack(kid->packed, e.state);

		unsigned long hash = d.hash(kid->packed);
		Node *dup = static_cast<Node*>(closed.find(kid->packed, hash));
		if (dup) {
			this->res.dups++;
			nodes->destruct(kid);
		} else {
			kid->update(kid->g, parent, op, e.revop);
			computeutil(kid);
			closed.add(kid, hash);
			open.push(kid);
		}
		return kinfo;
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = Cost(0);
		n0->d = d.d(s0);
		computeutil(n0);
		n0->op = n0->pop = D::Nop;
		n0->parent = NULL;
		n0->expct = 0;
		return n0;
	}

	// compututil computes the utility value of the given node
	// using corrected estimates of d and h.
	void computeutil(Node *n) {
		n->t = timeper * (lastdelay <= 0 ? 1 : lastdelay) * n->d;
		n->u = -(wf * (n->h + n->g) + wt * n->t);
	}

	// updateopen updates the utilities of all nodes on open and
	// reinitializes the heap every 2^i expansions.
	void updateopen() {
		if (this->res.expd < nextresort)
			return;

		double t = walltime();
		timeper = (t - lasttime) / (this->res.expd - lastexpd);
		lasttime = t;
		lastexpd = this->res.expd;

		lastdelay = avgdelay;

		nextresort *= 2;
		nresort++;
		reinitheap();
	}

	// Reinitheap reinitialize the heap property in O(n)
	// time while also updating the utilities.
	void reinitheap() {
		if (open.size() <= 0)
			return;

		for (long i = open.size()-1; i > (long) open.size()/2; i--) 
			computeutil(open.at(i));

		for (long i = (long) open.size()/2; i >= 0; i--) {
			computeutil(open.at(i));
			open.pushdown(i);
		}
	}

	// wf and wt are the cost and time weight respectively.
	double wf, wt;

	// expansion delay
	double lastdelay, avgdelay;

	// for nodes-per-second estimation
	double timeper, lasttime;
	long lastexpd;

	// for resorting the open list
	unsigned long nextresort;
	unsigned int nresort;

	BinHeap<Node, Node*> open;
 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed;
	Pool<Node> *nodes;
};
