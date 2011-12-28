#include "../search/search.hpp"
#include <boost/pool/object_pool.hpp>

template <class D> struct Astar : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	
	struct Node : SearchNode<D> {
		Cost f;
	
		static bool pred(Node *a, Node *b) {
			if (a->f == b->f)
				return a->g > b->g;
			return a->f < b->f;
		}

		static Cost prio(Node *n) { return n->f; }

		static Cost tieprio(Node *n) { return n->g; }
	};

	Astar(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), closed(30000001) {
		nodes = new boost::object_pool<Node>();
	}

	~Astar(void) {
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
				SearchAlgorithm<D>::res.goal(d, n);
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
			SearchAlgorithm<D>::res.gend++;
			considerkid(d, n, state, op);
		}
	}

	void considerkid(D &d, Node *pnode, State &pstate, Oper op) {
		Node *k = nodes->construct();
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
			open.pre_update(dup);
			dup->f = dup->f - dup->g + k->g;
			dup->update(k->g, pnode, op, d.revop(pstate, op));
			open.post_update(dup);
			nodes->destroy(k);
		} else {
			k->f = k->g + d.h(kstate);
			d.undo(pstate, u);
			dup->update(k->g, pnode, op, d.revop(pstate, op));
			closed.add(k, h);
			open.push(k);
		}
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = 0;
		n0->f = d.h(s0);
		n0->pop = n0->op = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	OpenList<Node, Node, Cost> open;
 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed;
	boost::object_pool<Node> *nodes;
};
