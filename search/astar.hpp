#include <boost/pool/object_pool.hpp>
#include <boost/optional.hpp>
#include "../incl/search.hpp"
#include "../structs/htable.hpp"
#include "../search/openlist.hpp"

template <class D, class Cost> struct Node {
	typename D::State state;
	typename D::Oper pop;
	Cost g, f;
	HtableEnt<Node> htent;
	Node *parent;
	int openind;

	Node(void) : openind(-1) {}

	static bool pred(Node *a, Node *b) {
		if (a->f == b->f)
			return a->g > b->g;
		return a->f < b->f;
	}

	static void setind(Node *n, int i) { n->openind = i; }
	static int getind(Node *n) { return n->openind; }
};

template <class D> struct Node <D, char> {
	typename D::State state;
	typename D::Oper pop;
	typename D::Cost g, f;
	HtableEnt<Node> htent;
	Node *parent;
	Node *n, *p;

	Node(void) : n(NULL), p(NULL) {}
	static typename D::Cost prio(Node *n) { return n->f; }
	static Node **nxt(Node *n) { return &n->n; }
	static Node **prev(Node *n) { return &n->p; }
};

template <class D> struct Node <D, short> : public Node <D, char> {};
template <class D> struct Node <D, int> : public Node <D, char> {};
template <class D> struct Node <D, long> : public Node <D, char> {};

template <class D, bool unitcost=false> class Astar : public Search<D> {
public:

	Result<D> search(D &d, typename D::State &s0) {
		res = Result<D>(true);

		Node<D, Cost> *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		while (!open.empty()) {
			Node<D, Cost> *n = open.pop();

			if (!unitcost && d.isgoal(n->state)) {
				handlesol(n);
				break;
			}

			if (expand(d, n))
				break;
		}

		res.finish();

		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", open.kind());
		dfpair(stdout, "node size", "%u", sizeof(Node<D, Cost>));

		return res;
	}

	Astar(void) : closed(30000001) {}

private:

	typedef typename D::State State;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	bool expand(D &d, Node<D, Cost> *n) {
		res.expd++;

		for (unsigned int i = 0; i < d.nops(n->state); i++) {
			Oper op = d.nthop(n->state, i);
			if (op == n->pop)
				continue;
			Node<D, Cost> *k = kid(d, n, op);
			res.gend++;

			if (unitcost && d.isgoal(k->state)) {
				handlesol(k);
				return true;
			}
				
			considerkid(d, k);
		}

		return false;
	}

	void considerkid(D &d, Node<D, Cost> *k) {
		unsigned long h = k->state.hash();
		Node<D, Cost> *dup = closed.find(k->state, h);
		if (dup) {
			res.dups++;
			if (k->g >= dup->g) {
				nodes.destroy(k);
				return;
			}
			if (!open.mem(dup))
				open.pre_update(dup);

			dup->f = dup->f - dup->g + k->g;
			dup->g = k->g;
			dup->pop = k->pop;
			dup->parent = k->parent;

			if (!open.mem(dup))
				open.push(dup);
			else
				open.post_update(dup);
			nodes.destroy(k);
		} else {
			closed.add(k, h);
			open.push(k);
		}
	}

	Node<D, Cost> *kid(D &d, Node<D, Cost> *parent, Oper op) {
		Node<D, Cost> *kid = nodes.construct();
		d.applyinto(kid->state, parent->state, op);
		kid->g = parent->g + d.opcost(parent->state, op);
		kid->f = kid->g + d.h(kid->state);
		kid->pop = d.revop(parent->state, op);
		kid->parent = parent;
		return kid;
	}

	Node<D, Cost> *init(D &d, State &s0) {
		Node<D, Cost> *n0 = nodes.construct();
		n0->state = s0;
		n0->g = 0;
		n0->f = d.h(s0);
		n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	void handlesol(Node<D, Cost> *n) {
		res.cost = n->g;

		for ( ; n; n = n->parent)
			res.path.push_back(n->state);
	}

	struct Closedops {
		static State &key(Node<D, Cost> *n) { return n->state; }
		static unsigned long hash(State &s) { return s.hash(); }
		static bool eq(State &a, State &b) { return a.eq(b); }
		static HtableEnt< Node<D, Cost> > &entry(Node<D, Cost> *n) {
			return n->htent;
		}
	};

	Result<D> res;
	OpenList< Node<D, Cost>, Node<D, Cost>, Cost > open;
 	Htable< Closedops, State&, Node<D, Cost> > closed;
	boost::object_pool< Node<D, Cost> > nodes;
};