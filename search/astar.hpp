#include "../search/search.hpp"
#include "../utils/pool.hpp"

template <class D> struct Astar : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
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
		nodes = new Pool<Node>();
	}

	~Astar(void) {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {
		this->start();
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
		this->finish();
	}

	virtual void reset(void) {
		SearchAlgorithm<D>::reset();
		open.clear();
		closed.clear();
		delete nodes;
		nodes = new Pool<Node>();
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

	void considerkid(D &d, Node *parent, State &state, Oper op) {
		Node *kid = nodes->construct();
		assert (kid);
		typename D::Transition tr(d, state, op);
		kid->g = parent->g + tr.cost;
		d.pack(kid->packed, tr.state);

		unsigned long hash = kid->packed.hash();
		Node *dup = static_cast<Node*>(closed.find(kid->packed, hash));
		if (dup) {
			SearchAlgorithm<D>::res.dups++;
			if (kid->g >= dup->g) {
				nodes->destruct(kid);
				return;
			}
			SearchAlgorithm<D>::res.reopnd++;
			open.pre_update(dup);
			dup->f = dup->f - dup->g + kid->g;
			dup->update(kid->g, parent, op, tr.revop);
			open.post_update(dup);
			nodes->destruct(kid);
		} else {
			kid->f = kid->g + d.h(tr.state);
			kid->update(kid->g, parent, op, tr.revop);
			closed.add(kid, hash);
			open.push(kid);
		}
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = Cost(0);
		n0->f = d.h(s0);
		n0->pop = n0->op = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	OpenList<Node, Node, Cost> open;
 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed;
	Pool<Node> *nodes;
};
