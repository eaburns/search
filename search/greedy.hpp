#include "../search/search.hpp"
#include <boost/pool/object_pool.hpp>

template <class D, bool speedy = false> struct Greedy : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node : SearchNode<D> {
		Cost h;

		static bool pred(Node *a, Node *b) { return a->h < b->h; }

		static typename D::Cost prio(Node *n) { return n->h; }

		static typename D::Cost tieprio(Node *n) { return n->g; }
	};

	Greedy(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), closed(30000001) {
		nodes = new boost::object_pool<Node>();
	}

	~Greedy(void) {
		delete nodes;
	}

	Result<D> &search(D &d, typename D::State &s0) {
		SearchAlgorithm<D>::res.start();
		closed.init(d);

		Node *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		while (!open.empty() && !SearchAlgorithm<D>::limit()) {
			Node *n = open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state)) {
				SearchAlgorithm<D>::res = Result<D>(d, n);
				break;
			}

			expand(d, n, state);
		}
		SearchAlgorithm<D>::res.finish();

		return SearchAlgorithm<D>::res;
	}

	virtual void reset(void) {
		SearchAlgorithm<D>::reset();
		open.clear();
		closed.clear();
		delete nodes;
		nodes = new boost::object_pool<Node>();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", open.kind());
		dfpair(stdout, "node size", "%u", sizeof(Node));
	}

private:

	void expand(D &d, Node *n, State &state) {
		SearchAlgorithm<D>::res.expd++;

		for (unsigned int i = 0; i < d.nops(state); i++) {
			Oper op = d.nthop(state, i);
			if (op == n->pop)
				continue;
			Node *k = kid(d, n, state, op);
			SearchAlgorithm<D>::res.gend++;

			considerkid(d, k);
		}
	}

	void considerkid(D &d, Node *k) {
		unsigned long h = k->packed.hash();
		SearchNode<D> *dup = closed.find(k->packed, h);
		if (dup) {
			SearchAlgorithm<D>::res.dups++;
			nodes->destroy(k);
		} else {
			closed.add(k, h);
			open.push(k);
		}
	}

	Node *kid(D &d, Node *pnode, State &pstate, Oper op) {
		Node *kid = nodes->construct();
		kid->op = op;
		kid->pop = d.revop(pstate, op);
		kid->parent = pnode;
		Undo u(pstate, op);
		State buf, &kidst = d.apply(buf, pstate, kid->g, op);
		kid->g += pnode->g;
		kid->h = speedy ? d.d(kidst) : d.h(kidst);
		d.pack(kid->packed, kidst);
		d.undo(pstate, u);

		return kid;
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = 0;
		n0->h = speedy ? d.d(s0) : d.h(s0);
		n0->op = n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	OpenList<Node, Node, Cost> open;
 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed;
	boost::object_pool<Node> *nodes;
};
