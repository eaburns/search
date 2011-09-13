#include <boost/pool/object_pool.hpp>
#include <boost/optional.hpp>
#include "../incl/search.hpp"
#include "../structs/htable.hpp"
#include "../search/openlist.hpp"

template <class D, class Cost> struct Node {
	typename D::PackedState state;
	typename D::Oper pop;
	Cost g, f;
	HtableEntry<Node> closedent;
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
	typename D::PackedState state;
	typename D::Oper pop;
	typename D::Cost g, f;
	HtableEntry<Node> closedent;
	IntpqEntry<Node> openent;
	Node *parent;

	static typename D::Cost prio(Node *n) { return n->f; }
	static IntpqEntry<Node> &openentry(Node *n) { return n->openent; }
};

template <class D> struct Node <D, short> : public Node <D, char> {};
template <class D> struct Node <D, int> : public Node <D, char> {};
template <class D> struct Node <D, long> : public Node <D, char> {};

template <class D> class Astar : public Search<D> {
public:

	Result<D> search(D &d, typename D::State &s0) {
		res = Result<D>(true);

		Node<D, Cost> *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		while (!open.empty()) {
			Node<D, Cost> *n = open.pop();

			State buf;
			State &state = d.unpack(buf, n->state);

			if (d.isgoal(state)) {
				handlesol(d, n);
				break;
			}

			if (expand(d, n, state))
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
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	bool expand(D &d, Node<D, Cost> *n, State &state) {
		res.expd++;

		for (unsigned int i = 0; i < d.nops(state); i++) {
			Oper op = d.nthop(state, i);
			if (op == n->pop)
				continue;
			Node<D, Cost> *k = kid(d, n, state, op);
			res.gend++;
				
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

	Node<D, Cost> *kid(D &d, Node<D, Cost> *pnode, State &pstate, Oper op) {
		Node<D, Cost> *kid = nodes.construct();
		State buf;
		State &kidst = d.applyinto(kid->state, buf, pstate, op);
		kid->g = pnode->g + d.opcost(pstate, op);
		kid->f = kid->g + d.h(kidst);
		kid->pop = d.revop(pstate, op);
		kid->parent = pnode;
		return kid;
	}

	Node<D, Cost> *init(D &d, State &s0) {
		Node<D, Cost> *n0 = nodes.construct();
		d.pack(n0->state, s0);
		n0->g = 0;
		n0->f = d.h(s0);
		n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	void handlesol(D &d, Node<D, Cost> *n) {	
		res.cost = n->g;

		for ( ; n; n = n->parent) {
			State buf;
			State &state = d.unpack(buf, n->state);
			res.path.push_back(state);
		}
	}

	struct Closedops {
		static PackedState &key(Node<D, Cost> *n) { return n->state; }
		static unsigned long hash(PackedState &s) { return s.hash(); }
		static bool eq(PackedState &a, PackedState &b) { return a.eq(b); }
		static HtableEntry< Node<D, Cost> > &entry(Node<D, Cost> *n) {
			return n->closedent;
		}
	};

	Result<D> res;
	OpenList< Node<D, Cost>, Node<D, Cost>, Cost > open;
 	Htable< Closedops, PackedState&, Node<D, Cost> > closed;
	boost::object_pool< Node<D, Cost> > nodes;
};