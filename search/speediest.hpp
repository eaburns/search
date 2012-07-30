#include "../search/search.hpp"
#include "../utils/pool.hpp"

template <class D> struct Speediest : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node : SearchNode<D> {
		Cost d;
		double sortval;
		unsigned int depth;

		static bool pred(Node *a, Node *b) { return a->sortval < b->sortval; }

		static typename D::Cost prio(Node *n) { return n->sortval; }

		static typename D::Cost tieprio(Node *n) { return n->g; }
	};

	Speediest(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), closed(30000001) {
		nodes = new Pool<Node>();
	}

	~Speediest() {
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

	virtual void reset() {
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

		typename D::Operators ops(d, state);
		for (unsigned int i = 0; i < ops.size(); i++) {
			if (ops[i] == n->pop)
				continue;
			SearchAlgorithm<D>::res.gend++;
			considerkid(d, n, state, ops[i]);
		}
	}

	void considerkid(D &d, Node *parent, State &state, Oper op) {
		Node *kid = nodes->construct();
		typename D::Edge e(d, state, op);
		d.pack(kid->packed, e.state);

		unsigned long hash = d.hash(kid->packed);
		SearchNode<D> *dup = closed.find(kid->packed, hash);
		if (dup) {
			this->res.dups++;
			nodes->destruct(kid);
		} else {
			kid->g = parent->g + e.cost;
			kid->depth = parent->depth + 1;
			kid->d = d.d(e.state);
			kid->sortval = kid->d / (kid->d + kid->depth);
			kid->update(kid->g, parent, op, e.revop);
			closed.add(kid, hash);
			open.push(kid);
		}
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = Cost(0);
		n0->d = d.d(s0);
		n0->sortval = 1;
		n0->depth = 0;
		n0->op = n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	OpenList<Node, Node, Cost> open;
 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed;
	Pool<Node> *nodes;
};
