#include "../incl/search.hpp"
#include "../structs/htable.hpp"
#include "../structs/intpq.hpp"
#include "../structs/binheap.hpp"
#include <boost/pool/object_pool.hpp>

template <class D, class Cost> struct Node {
	typename D::State state;
	typename D::Oper pop;
	Cost g, f;
	Node *htblnxt, *parent;
	int openind;

	Node(void) : openind(-1) {}
};

template <class D, class Cost> class OpenList {
public:
	const char *kind(void) { return "binary heap"; }
	void push(Node<D, Cost> *n) { heap.push(n); }
	Node<D, Cost> *pop(void) { return heap.pop(); }
	void pre_update(Node<D, Cost> *n) { }
	void post_update(Node<D, Cost> *n) { heap.update(n->openind); }
	bool empty(void) { return heap.empty(); }
	bool mem(Node<D, Cost> *n) { return n->openind != -1; }
private:
	struct Ops {
 			static bool pred(Node<D, Cost> *a, Node<D, Cost> *b) {
			if (a->f == b->f)
				return a->g > b->g;
			return a->f < b->f;
		}
		static void setind(Node<D, Cost> *n, int i) { n->openind = i; }
	};
	Binheap< Ops, Node<D, Cost> > heap;
};

template <class D> struct Node <D, char> {
	typename D::State state;
	typename D::Oper pop;
	typename D::Cost g, f;
	Node *htblnxt, *parent;
	Node *nxt, *prev;

	Node(void) : nxt(NULL), prev(NULL) {}
};

template <class D> class OpenList <D, char> {
private:
	typedef typename D::Cost Cost;
public:
	static const char *kind(void) { return "bucketed"; }
	void push(Node<D, Cost> *n) { pq.push(n, n->f); }
	Node<D, Cost> *pop(void) { return pq.pop(); }
	void pre_update(Node<D, Cost> *n) { pq.rm(n); }
	void post_update(Node<D, Cost> *n) { push(n); }
	bool empty(void) { return pq.empty(); }
	bool mem(Node<D, Cost> *n) { return pq.mem(n); }
private:
	struct Ops {
		static Node<D, Cost> **nxt(Node<D, Cost> *n) { return &n->nxt; }
		static Node<D, Cost> **prev(Node<D, Cost> *n) { return &n->prev; }
	};
	Intpq< Ops, Node<D, Cost> > pq;
};

template <class D> class Astar : public Search<D> {
public:

	Result<D> search(D &d, typename D::State &s0) {
		res = Result<D>(true);

		Node<D, Cost> *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		while (!open.empty()) {
			Node<D, Cost> *n = open.pop();

			if (d.isgoal(n->state)) {
				handlesol(n);
				break;
			}

			expand(d, n);
		}

		res.finish();
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", open.kind());

		return res;
	}

	Astar(void) : closed(30000001) {}

private:

	typedef typename D::State State;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;


	void expand(D &d, Node<D, Cost> *n) {
		res.expd++;
		for (unsigned int i = 0; i < d.nops(n->state); i++) {
			Oper op = d.nthop(n->state, i);
			if (op == n->pop)
				continue;
			Node<D, Cost> *k = kid(d, n, op);
			res.gend++;
			considerkid(d, k);
		}
	}

	void considerkid(D &d, Node<D, Cost> *k) {
		unsigned long h = k->state.hash();
		Node<D, Cost> *dup = closed.find(&k->state, h);
		if (dup) {
			res.dups++;
			if (k->g >= dup->g) {
				nodes.free(k);
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
			nodes.free(k);
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
		static State *key(Node<D, Cost> *n) { return &n->state; }
		static unsigned long hash(State *s) { return s->hash(); }
		static bool eq(State *a, State *b) { return a->eq(*b); }
		static Node<D, Cost> **nxt(Node<D, Cost> *n) { return &n->htblnxt; }
	};

	Result<D> res;
	OpenList<D, Cost> open;
 	Htable< Closedops, State*, Node<D, Cost> > closed;
	boost::object_pool< Node<D, Cost> > nodes;
};