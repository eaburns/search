#include "../search/search.hpp"
#include "../structs/binheap.hpp"
#include <boost/pool/object_pool.hpp>
#include <limits>
#include <vector>

void dfrowhdr(FILE *, const char *name, int ncols, ...);
void dfrow(FILE *, const char *name, const char *colfmt, ...);
void fatal(const char*, ...);

template <class D> struct Arastar : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node : SearchNode<D> {
		Cost h;
		double fprime;
	
		static bool pred(Node *a, Node *b) {
			if (a->fprime == b->fprime)
				return a->g > b->g;
			return a->fprime < b->fprime;
		}
	};
	
	struct Incons {
		Incons(unsigned long szhint) : mem(szhint) { }
	
		void init(D &d) { mem.init(d); }
	
		void add(Node *n, unsigned long h) {
			if (mem.find(n->packed, h) != NULL)
				return;
			mem.add(n);
			incons.push_back(n);
		}
	
		const std::vector<Node*> &nodes(void) { return incons; }
	
		void clear(void) {
			mem.clear();
			incons.clear();
		}
	
	private:
	 	ClosedList<SearchNode<D>, SearchNode<D>, D> mem;
		std::vector<Node*> incons;
	};

	Arastar(int argc, const char *argv[]) :
			SearchAlgorithm<D>(argc, argv), closed(30000001),
			incons(30000001) {

		wt0 = dwt = -1;
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-wt0") == 0)
				wt0 = strtod(argv[++i], NULL);
			else if (i < argc - 1 && strcmp(argv[i], "-dwt") == 0)
				dwt = strtod(argv[++i], NULL);
		}

		if (wt0 < 1)
			fatal("Must specify initial weight ≥ 1 using -wt0");
		if (dwt <= 0)
			fatal("Must specify weight decrement > 0 using -dwt");

		wt = wt0;
		nodes = new boost::object_pool<Node>();
	}

	~Arastar(void) {
		delete nodes;
	}

	Result<D> &search(D &d, typename D::State &s0) {
		SearchAlgorithm<D>::res.start();
		closed.init(d);
		incons.init(d);

		dfrowhdr(stdout, "sol", 6, "num", "nodes expanded",
			"nodes generated", "solution bound", "solution cost",
			"wall time");


		Node *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		unsigned long n = 0;
		do {
			improve(d);
			// No solution
			if (SearchAlgorithm<D>::res.cost == D::InfCost)
				break;

			n++;
			double epsprime = findbound();
			if (epsprime < wt)
				wt = epsprime;

			dfrow(stdout, "sol", "uuuggg", n,
				SearchAlgorithm<D>::res.expd,
				SearchAlgorithm<D>::res.gend, wt,
				(double) SearchAlgorithm<D>::res.cost,
				walltime() - SearchAlgorithm<D>::res.wallstrt);

			if (wt <= 1.0)
				break;

			wt = wt - dwt > 1.0 ? wt - dwt : 1.0;

			updateopen();
			closed.clear();

		} while(!SearchAlgorithm<D>::limit() && !open.empty());

		SearchAlgorithm<D>::res.finish();
		return SearchAlgorithm<D>::res;
	}

	virtual void reset(void) {
		SearchAlgorithm<D>::reset();
		wt = wt0;
		open.clear();
		closed.clear();
		incons.clear();
		delete nodes;
		nodes = new boost::object_pool<Node>();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", "binary heap");
		dfpair(stdout, "node size", "%u", sizeof(Node));
		dfpair(stdout, "initial weight", "%g", wt0);
		dfpair(stdout, "weight decrement", "%g", dwt);
	}

private:

	void improve(D &d) {
		while (!SearchAlgorithm<D>::limit() && goodnodes()) {
			Node *n = *open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state))
				SearchAlgorithm<D>::res.goal(d, n);

			expand(d, n, state);
		}
	}

	bool goodnodes() {
		return !open.empty() &&
			(SearchAlgorithm<D>::res.cost == D::InfCost ||
			SearchAlgorithm<D>::res.cost > (*open.front())->fprime);
	}

	// Find the tightest bound for the current incumbent.
	double findbound(void) {
		assert (SearchAlgorithm<D>::res.cost != D::InfCost);
		double min = std::numeric_limits<double>::infinity();

		for (unsigned int i = 0; i < incons.nodes().size(); i++) {
			Node *n = incons.nodes()[i];
			double f = n->g + n->h;
			if (f < min)
				min = f;
		}
		for (unsigned int i = 0; i < open.size(); i++) {
			Node *n = open.at(i);
			double f = n->g + n->h;
			if (f < min)
				min = f;
		}
		return (double) SearchAlgorithm<D>::res.cost / min;
	}

	// Update the open list: update f' values and add INCONS
	// and re-heapify.
	void updateopen(void) {
		for (unsigned int i = 0; i < incons.nodes().size(); i++) {
			Node *n = incons.nodes()[i];
			n->fprime = n->g + wt * n->h;
		}
		for (unsigned int i = 0; i < open.size(); i++) {
			Node *n = open.at(i);
			n->fprime = n->g + wt * n->h;
		}
		open.append(incons.nodes());	// reinits heap property
		incons.clear();
	}

	void expand(D &d, Node *n, State &state) {
		SearchAlgorithm<D>::res.expd++;
		for (unsigned int i = 0; i < d.nops(state); i++) {
			Oper op = d.nthop(state, i);
			if (op == n->pop)
				continue;

			SearchAlgorithm<D>::res.gend++;
			considerkid(d, n, state, op);
		}
	}

	void considerkid(D &d, Node *pnode, State &pstate, Oper op) {
		Node *k = nodes->construct();
		k->op = op;
		k->pop = d.revop(pstate, op);
		k->parent = pnode;
		Undo u(pstate, op);
		State buf, &kstate = d.apply(buf, pstate, k->g, op);
		k->g += pnode->g;
		d.pack(k->packed, kstate);

		unsigned long h = k->packed.hash();
		Node *dup = static_cast<Node*>(closed.find(k->packed, h));
		if (dup) {
			d.undo(pstate, u);
			SearchAlgorithm<D>::res.dups++;
			if (k->g >= dup->g) {
				nodes->destroy(k);
				return;
			}

			SearchAlgorithm<D>::res.reopnd++;
			dup->fprime = dup->fprime - dup->g + k->g;
			dup->update(*k);

			if (dup->openind < 0)
				incons.add(dup, h);
			else
				open.update(dup->openind);
			nodes->destroy(k);
		} else {
			k->h = d.h(kstate);
			k->fprime = k->g + wt * k->h;
			d.undo(pstate, u);
			closed.add(k, h);
			open.push(k);
		}
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = 0;
		n0->h = d.h(s0);
		n0->fprime = wt * n0->h;
		n0->op = n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	double wt0, dwt, wt;
	BinHeap<Node, Node*> open;
 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed;
	Incons incons;
	boost::object_pool<Node> *nodes;
};
