#include "../search/search.hpp"
#include "../structs/htable.hpp"
#include "../structs/binheap.hpp"
#include <boost/pool/object_pool.hpp>
#include <limits>
#include <vector>

void dfrowhdr(FILE *, const char *name, int ncols, ...);
void dfrow(FILE *, const char *name, const char *colfmt, ...);
void fatal(const char*, ...);

template <class D> struct ArastarNode {
	typedef ArastarNode Node;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef typename D::PackedState PackedState;

	HtableEntry<Node> closedent;
	PackedState packed;
	Oper pop;
	Cost g, h;
	double fprime;
	Node *parent;
	long openind;

	ArastarNode(void) : openind(-1) {}

	static bool pred(Node *a, Node *b) {
		if (a->fprime == b->fprime)
			return a->g > b->g;
		return a->fprime < b->fprime;
	}

	static void setind(Node *n, int i) { n->openind = i; }

	static PackedState &key(Node *n) { return n->packed; }

	static unsigned long hash(PackedState &s) { return s.hash(); }

	static bool eq(PackedState &a, PackedState &b) { return a.eq(b); }

	static HtableEntry<Node> &entry(Node *n) { return n->closedent; }
};

template <class Ops, class Key, class Node> struct Incons {

	void add(Node *n) {
		if (mem.find(n->packed) != NULL)
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
	Htable<Ops, Key, Node, 0> mem;
	std::vector<Node*> incons;
};

template <class D> struct Arastar : public Search<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef ArastarNode<D> Node;

	Arastar(int argc, char *argv[]) :
			Search<D>(argc, argv), closed(30000001) {

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
		Search<D>::res.start();
		Node *n0 = init(d, s0);
		unsigned long n = 0;

		dfrowhdr(stdout, "sol", 6, "num", "nodes expanded",
			"nodes generated", "solution bound", "solution cost",
			"wall time");

		closed.add(n0);
		open.push(n0);

		do {
			improve(d);
			if (Search<D>::res.cost == D::InfCost)	// no solution
				break;

			n++;
			wt = findbound();

			dfrow(stdout, "sol", "uuuggg", n, Search<D>::res.expd,
				Search<D>::res.gend, wt, (double) Search<D>::res.cost,
				walltime() - Search<D>::res.wallstrt);

			if (wt <= 1.0)
				break;

			wt = wt - dwt > 1.0 ? wt - dwt : 1.0;

			updateopen();
			closed.clear();

		} while(!Search<D>::limit() && !open.empty());

		Search<D>::res.finish();
		return Search<D>::res;
	}

	virtual void reset(void) {
		Search<D>::reset();
		wt = wt0;
		open.clear();
		closed.clear();
		incons.clear();
		delete nodes;
		nodes = new boost::object_pool<Node>();
	}

	virtual void output(FILE *out) {
		Search<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", "binary heap");
		dfpair(stdout, "node size", "%u", sizeof(Node));
		dfpair(stdout, "initial weight", "%g", wt0);
		dfpair(stdout, "weight decrement", "%g", dwt);
	}

private:

	void improve(D &d) {
		while (!Search<D>::limit() && goodnodes()) {
			Node *n = *open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state))
				handlesol(d, n);

			expand(d, n, state);
		}
	}

	bool goodnodes() {
		return !open.empty() &&
			(Search<D>::res.cost == D::InfCost ||
			Search<D>::res.cost > (*open.front())->fprime);
	}

	// Find the tightest bound for the current incumbent.
	double findbound(void) {
		assert (Search<D>::res.cost != D::InfCost);
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
		return (double) Search<D>::res.cost / min;
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
		Search<D>::res.expd++;
		for (unsigned int i = 0; i < d.nops(state); i++) {
			Oper op = d.nthop(state, i);
			if (op == n->pop)
				continue;

			Node *k = kid(d, n, state, op);
			Search<D>::res.gend++;

			considerkid(d, k);
		}
	}

	void considerkid(D &d, Node *k) {
		unsigned long h = k->packed.hash();
		Node *dup = closed.find(k->packed, h);
		if (dup) {
			Search<D>::res.dups++;
			if (k->g >= dup->g) {
				nodes->destroy(k);
				return;
			}

			Search<D>::res.reopnd++;
			dup->fprime = dup->fprime - dup->g + k->g;
			dup->g = k->g;
			dup->pop = k->pop;
			dup->parent = k->parent;

			if (dup->openind < 0)
				incons.add(dup);
			else
				open.update(dup->openind);
			nodes->destroy(k);
		} else {
			closed.add(k, h);
			open.push(k);
		}
	}

	Node *kid(D &d, Node *pnode, State &pstate, Oper op) {
		Node *kid = nodes->construct();
		kid->g = pnode->g + d.opcost(pstate, op);
		kid->pop = d.revop(pstate, op);
		kid->parent = pnode;
		Undo u(pstate, op);
		State buf, &kidst = d.apply(buf, pstate, op);
		kid->h = d.h(kidst);
		kid->fprime = kid->g + wt * kid->h;
		d.pack(kid->packed, kidst);
		d.undo(pstate, u);

		return kid;
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = 0;
		n0->h = d.h(s0);
		n0->fprime = wt * n0->h;
		n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	void handlesol(D &d, Node *n) {
		Search<D>::res.cost = n->g;
		for ( ; n; n = n->parent) {
			State buf;
			State &state = d.unpack(buf, n->packed);
			Search<D>::res.path.push_back(state);
		}
	}

	double wt0, dwt, wt;

	BinHeap<Node, Node*> open;
 	Htable<Node, PackedState&, Node, 0> closed;
	Incons<Node, PackedState&, Node> incons;
	boost::object_pool<Node> *nodes;
};
