#include "../search/search.hpp"
#include "../search/closedlist.hpp"
#include "../search/openlist.hpp"
#include <boost/pool/object_pool.hpp>

void fatal(const char*, ...);	// utils.hpp

template <class D> struct WastarNode {
	ClosedEntry<WastarNode, D> closedent;
	typename D::PackedState packed;
	typename D::Oper pop;
	double g, f, fprime;
	WastarNode *parent;
	int openind;

	WastarNode(void) : openind(-1) {}

	static bool pred(WastarNode *a, WastarNode *b) {
		if (a->fprime == b->fprime) {
			if (a->f == b->f)
				return a->g > b->g;
			return a->f < b->f;
		}
		return a->fprime < b->fprime;
	}

	static void setind(WastarNode *n, int i) { n->openind = i; }

	static int getind(WastarNode *n) { return n->openind; }
};

template <class D> struct Wastar : public Search<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Oper Oper;
	typedef WastarNode<D> Node;

	Wastar(int argc, const char *argv[]) :
			Search<D>(argc, argv), wt(-1.0), closed(30000001) {
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-wt") == 0)
				wt = strtod(argv[++i], NULL);
		}

		if (wt < 1)
			fatal("Must specify a weight ≥1 weight using -wt");

		nodes = new boost::object_pool<Node>();
	}

	~Wastar(void) {
		delete nodes;
	}

	Result<D> &search(D &d, typename D::State &s0) {
		Search<D>::res.start();
		closed.init(d);

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
		dfpair(stdout, "weight", "%g", wt);
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
			if (k->g >= dup->g) {
				nodes->destroy(k);
				return;
			}
			Search<D>::res.reopnd++;
			if (open.mem(dup))
				open.pre_update(dup);

			dup->fprime = dup->fprime - dup->g + k->g;
			dup->f = dup->f - dup->g + k->g;
			dup->g = k->g;
			dup->pop = k->pop;
			dup->parent = k->parent;

			if (!open.mem(dup))
				open.push(dup);
			else
				open.post_update(dup);
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
		double h = d.h(kidst);
		kid->f = kid->g + h;
		kid->fprime = kid->g + wt * h;
		d.pack(kid->packed, kidst);
		d.undo(pstate, u);

		return kid;
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = 0;
		double h = d.h(s0);
		n0->f = h;
		n0->fprime = wt * h;
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
			if (n->parent)
				Search<D>::res.ops.push_back(n->pop);
		}
	}

	struct ClosedOps {
		static PackedState &key(Node *n) { return n->packed; }
		static unsigned long hash(PackedState &s) { return s.hash(); }
		static bool eq(PackedState &a, PackedState &b) { return a.eq(b); }
		static ClosedEntry<Node, D> &entry(Node *n) { return n->closedent; }
	};

	double wt;
	OpenList<Node, Node, double> open;
 	ClosedList<ClosedOps, Node, D> closed;
	boost::object_pool<Node> *nodes;
};
