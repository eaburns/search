#include "../incl/search.hpp"
#include "../structs/htable.hpp"
#include "openlist.hpp"
#include <boost/pool/object_pool.hpp>
#include <boost/optional.hpp>

template <class D> class Astar : public Search<D> {
public:

	Result<D> search(D &d, typename D::State &s0) {
		res = Result<D>(true);

		Node *n0 = init(d, s0);
		closed.add(&n0->key, n0);
		open.push(n0);

		while (!open.empty()) {
			Node *n = *open.pop();

			if (d.isgoal(n->state)) {
				handlesol(n);
				break;
			}

			expand(d, n);
		}

		res.finish();
		return res;
	}

private:

	typedef typename D::State State;
	typedef typename D::State::Key Key;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node {
		State state;
		Key key;
		Cost g, f;
		Oper pop;
		Node *parent;
		int openind;
	};

	void expand(D &d, Node *n) {
		res.expd++;
		for (unsigned int i = 0; i < d.nops(n->state); i++) {
			Oper op = d.nthop(n->state, i);
			if (op == n->pop)
				continue;
			Node *k = kid(d, n, op);
			res.gend++;
			considerkid(d, k);
		}
	}

	void considerkid(D &d, Node *k) {
		boost::optional<Node*> dupopt = closed.find(&k->key);
		if (dupopt) {
			res.dups++;
			Node *dup = *dupopt;
			if (k->g >= dup->g) {
				nodes.free(k);
				return;
			}
			if (!open.mem(dup)) {
				open.push(dup);
			} else {
				open.pre_update(dup);
				dup->f = dup->f - dup->g + k->g;
				dup->g = k->g;
				open.post_update(dup);
			}
			nodes.free(k);
		} else {
			closed.add(&k->key, k);
			open.push(k);
		}
	}

	Node *kid(D &d, Node *parent, Oper op) {
		Node *kid = nodes.malloc();
		d.applyinto(kid->state, parent->state, op);
		kid->key = kid->state.key();
		kid->g = parent->g + d.opcost(parent->state, op);
		kid->f = kid->g + d.h(kid->state);
		kid->pop = d.revop(parent->state, op);
		kid->parent = parent;
		kid->openind = OpenList<Openops, Node*, Cost>::InitInd;
		return kid;
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes.malloc();
		n0->state = s0;
		n0->key = s0.key();
		n0->g = 0;
		n0->f = d.h(s0);
		n0->pop = D::Nop;
		n0->parent = NULL;
		n0->openind = OpenList<Openops, Node*, Cost>::InitInd;
		return n0;
	}

	void handlesol(Node *n) {
		res.cost = n->g;

		for ( ; n; n = n->parent)
			res.path.push_back(n->state);
	}

	struct Openops {
		static Cost prio(Node *n) {
			return n->f;
		}

		static bool pred(Node *a, Node *b) {
			if (a->f == b->f)
				return a->g > b->g;
			return a->f < b->f;
		}

		static void setind(Node *n, int i) {
			n->openind = i;
		}

		static int getind(Node *n) {
			return n->openind;
		}
	};

	struct Closedops {
		static unsigned long hash(Key *k) {
			return k->hash();
		}
		static bool eq(Key *a, Key *b) {
			return a->eq(*b);
		}
	};

	Result<D> res;
	OpenList<Openops, Node*, Cost> open;
 	Htable<Closedops, Key*, Node*> closed;
	boost::object_pool<Node> nodes;
};