#include "../incl/search.hpp"
#include "../structs/htable.hpp"
#include "../search/openlist.hpp"
#include "../utils/pool.hpp"

template <class D, class Cost> struct Node {
	typename D::PackedState packed;
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
	typename D::PackedState packed;
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
		Node<D, Cost> *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		while (!open.empty() && !Search<D>::limit()) {
			Node<D, Cost> *n = open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state)) {
				handlesol(d, n);
				break;
			}

			expand(d, n, state);
		}
		Search<D>::res.finish();

		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", open.kind());
		dfpair(stdout, "node size", "%u", sizeof(Node<D, Cost>));

		return Search<D>::res;
	}

	Astar(int argc, char *argv[]) :
		Search<D>(argc, argv),
		closed(30000001),
		nodes(35000000) {}

private:

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	void expand(D &d, Node<D, Cost> *n, State &state) {
		Search<D>::res.expd++;

		for (unsigned int i = 0; i < d.nops(state); i++) {
			Oper op = d.nthop(state, i);
			if (op == n->pop)
				continue;
			Node<D, Cost> *k = kid(d, n, state, op);
			Search<D>::res.gend++;

			considerkid(d, k);
		}
	}

	void considerkid(D &d, Node<D, Cost> *k) {
		unsigned long h = k->packed.hash();
		Node<D, Cost> *dup = closed.find(k->packed, h);
		if (dup) {
			Search<D>::res.dups++;
			if (k->g >= dup->g) {
				nodes.destruct(k);
				return;
			}
			Search<D>::res.reopnd++;
			if (open.mem(dup))
				open.pre_update(dup);

			dup->f = dup->f - dup->g + k->g;
			dup->g = k->g;
			dup->pop = k->pop;
			dup->parent = k->parent;

			if (!open.mem(dup))
				open.push(dup);
			else
				open.post_update(dup);
			nodes.destruct(k);
		} else {
			closed.add(k, h);
			open.push(k);
		}
	}

	Node<D, Cost> *kid(D &d, Node<D, Cost> *pnode, State &pstate, Oper op) {
		Node<D, Cost> *kid = nodes.construct();

		kid->g = pnode->g + d.opcost(pstate, op);
		kid->pop = d.revop(pstate, op);
		kid->parent = pnode;

		Undo u(pstate, op);
		State buf, &kidst = d.apply(buf, pstate, op);
		kid->f = kid->g + d.h(kidst);
		d.pack(kid->packed, kidst);
		d.undo(pstate, u);

		return kid;
	}

	Node<D, Cost> *init(D &d, State &s0) {
		Node<D, Cost> *n0 = nodes.construct();
		d.pack(n0->packed, s0);
		n0->g = 0;
		n0->f = d.h(s0);
		n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	void handlesol(D &d, Node<D, Cost> *n) {
		Search<D>::res.cost = n->g;

		for ( ; n; n = n->parent) {
			State buf;
			State &state = d.unpack(buf, n->packed);
			Search<D>::res.path.push_back(state);
		}
	}

	struct Closedops {
		static PackedState &key(Node<D, Cost> *n) { return n->packed; }
		static unsigned long hash(PackedState &s) { return s.hash(); }
		static bool eq(PackedState &a, PackedState &b) { return a.eq(b); }
		static HtableEntry< Node<D, Cost> > &entry(Node<D, Cost> *n) {
			return n->closedent;
		}
	};

	OpenList< Node<D, Cost>, Node<D, Cost>, Cost > open;
 	Htable< Closedops, PackedState&, Node<D, Cost>, 0 > closed;
	Pool< Node<D, Cost> > nodes;
};
