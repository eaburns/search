// Best-first Utility-Guided Search Yes!

#include "../search/search.hpp"
#include "../structs/binheap.hpp"
#include "../utils/pool.hpp"

template <class D> struct Bugsy_slim : public SearchAlgorithm<D> {
	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef typename D::Edge Edge;

	struct Node {

		ClosedEntry<Node, D> closedent;
		int openind;
		Node *parent;
		PackedState state;
		Oper op, pop;
		Cost g, h, d;
		double u, t;

		// The expansion count when this node was
		// generated.
		unsigned long expct;

		Node() : openind(-1) {
		}

		static ClosedEntry<Node, D> &closedentry(Node *n) {
			return n->closedent;
		}

		static PackedState &key(Node *n) {
			return n->state;
		}

		static void setind(Node *n, int i) {
			n->openind = i;
		}

		static int getind(const Node *n) {
			return n->openind;
		}

		static bool pred(Node *a, Node *b) {
			if (a->u != b->u)
				return a->u > b->u;
			if (a->t != b->t)
				return a->t < b->t;

			Cost af = a->g + a->h;
			Cost bf = b->g + b->h;
			if (af != bf)
				return af < bf;

			return a->g > b->g;
		}
	};

	enum {
		// Resort1 is the number of expansions to perform
		// before the 1st open list resort.
		Resort1 = 128,
	};

	Bugsy_slim(int argc, const char *argv[]) :
			SearchAlgorithm<D>(argc, argv), avgdelay(0), delaysum(0),
			timeper(0.0), lastexpd(0), nextresort(Resort1), nresort(0),
			closed(30000001) {
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

	}

	~Bugsy_slim() {
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
			State buf, &state = d.unpack(buf, n->state);
			if (d.isgoal(state)) {
				solpath<D, Node>(d, n, this->res);
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
		for (auto n : closed)
			nodes.destruct(n);
		timeper = 0.0;
		lastexpd = 0;
		delaysum = 0;
		avgdelay = 0;
		nresort = 0;
		nextresort = Resort1;
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

	// expand expands the node, adding its children to the
	// open and closed lists as appropriate.
	void expand(D &d, Node *n, State &state) {
		this->res.expd++;

		delaysum += this->res.expd - n->expct;

		typename D::Operators ops(d, state);
		for (unsigned int i = 0; i < ops.size(); i++) {
			if (ops[i] == n->pop)
				continue;

			this->res.gend++;
			Edge e(d, state, ops[i]);

			Node *kid = nodes.construct();
			d.pack(kid->state, e.state);

			unsigned long hash = kid->state.hash(&d);
			if (closed.find(kid->state, hash)) {
				this->res.dups++;
				nodes.destruct(kid);
				continue;
			}

			kid->expct = this->res.expd;
			kid->g = n->g + e.cost;
			kid->d = std::max(d.d(e.state), n->d - Cost(1));
			kid->h = std::max(d.h(e.state), n->h - e.cost);
			kid->parent = n;
			kid->op = ops[i];
			kid->pop = e.revop;
			computeutil(kid);
			closed.add(kid, hash);
			open.push(kid);
		}
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes.construct();
		d.pack(n0->state, s0);
		n0->g = Cost(0);
		n0->h = d.h(s0);
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
		n->t = timeper * (avgdelay <= 0 ? 1 : avgdelay) * n->d;
		n->u = -(wf * (n->h + n->g) + wt * n->t);
	}

	// updateopen updates the utilities of all nodes on open and
	// reinitializes the heap every 2^i expansions.
	void updateopen() {
		if (this->res.expd < nextresort)
			return;

		double nexpd = this->res.expd - lastexpd;
		lastexpd = this->res.expd;

		double t = walltime();
		timeper = (t - lasttime) / nexpd;
		lasttime = t;

		avgdelay = delaysum/nexpd;
		delaysum = 0;

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
	double avgdelay;
	unsigned long delaysum;

	// for nodes-per-second estimation
	double timeper, lasttime;
	long lastexpd;

	// for resorting the open list
	unsigned long nextresort;
	unsigned int nresort;

	BinHeap<Node, Node*> open;
 	ClosedList<Node, Node, D> closed;
	Pool<Node> nodes;
};
