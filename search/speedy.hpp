#include "../search/search.hpp"
#include "../structs/htable.hpp"
#include "../search/openlist.hpp"
#include <boost/pool/object_pool.hpp>

template <class D, class Cost> struct SpeedyNode {
	typename D::PackedState packed;
	typename D::Oper pop;
	Cost g, d;
	HtableEntry<SpeedyNode> closedent;
	SpeedyNode *parent;

	SpeedyNode(void) {}

	static bool pred(SpeedyNode *a, SpeedyNode *b) {
		if (a->d == b->d)
			return a->g > b->g;
		return a->d < b->d;
	}

	static void setind(SpeedyNode*, int) { }

	static int getind(SpeedyNode*) { return -1; }
};

template <class D> struct SpeedyNode <D, IntOpenCost> {
	typename D::PackedState packed;
	typename D::Oper pop;
	typename D::Cost g, d;
	HtableEntry<SpeedyNode> closedent;
	OpenEntry<SpeedyNode> openent;
	SpeedyNode *parent;

	static typename D::Cost prio(SpeedyNode *n) { return n->d; }

	static OpenEntry<SpeedyNode> &openentry(SpeedyNode *n) {
		return n->openent;
	}
};

template <class D> struct Speedy : public Search<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef SpeedyNode<D, Cost> Node;

	Speedy(int argc, char *argv[]) :
		Search<D>(argc, argv), closed(30000001) {
		nodes = new boost::object_pool<Node>();
	}

	~Speedy(void) {
		delete nodes;
	}

	Result<D> &search(D &d, typename D::State &s0) {
		Search<D>::res.start();

		Node *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		while (!open.empty() && !Search<D>::limit()) {
			Node *n = open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state)) {
				handlesol(d, n);
				break;
			}

			expand(d, n, state);
		}
		Search<D>::res.finish();

		return Search<D>::res;
	}

	virtual void reset(void) {
		Search<D>::reset();
		open.clear();
		closed.clear();
		delete nodes;
		nodes = new boost::object_pool<Node>();
	}

	virtual void output(FILE *out) {
		Search<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", open.kind());
		dfpair(stdout, "node size", "%u", sizeof(Node));
	}

private:

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
		kid->d = d.d(kidst);
		d.pack(kid->packed, kidst);
		d.undo(pstate, u);

		return kid;
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = 0;
		n0->d = d.d(s0);
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

	struct Closedops {
		static PackedState &key(Node *n) { return n->packed; }
		static unsigned long hash(PackedState &s) { return s.hash(); }
		static bool eq(PackedState &a, PackedState &b) { return a.eq(b); }
		static HtableEntry<Node> &entry(Node *n) {
			return n->closedent;
		}
	};

	OpenList<Node, Node, Cost> open;
 	Htable<Closedops, PackedState&, Node, 0> closed;
	boost::object_pool<Node> *nodes;
};
