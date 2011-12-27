// Best-first Utility-Guided Search Yes! From:
//"Best-first Utility-Guided Search," Wheeler Ruml and Minh B. Do,
//Proceedings of the Twentieth International Joint Conference on
//Artificial Intelligence (IJCAI-07), 2007

#include "../search/search.hpp"
#include "../structs/binheap.hpp"
#include <boost/pool/object_pool.hpp>

void fatal(const char*, ...);	// utils.hpp

template <class D> struct Bugsy : public SearchAlgorithm<D> {
	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Undo Undo;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node : SearchNode<D> {
		typename D::Cost f, h, d;
		double u, t;

		static bool pred(Node *a, Node *b) {
			if (a->u != b->u)
				return a->u > b->u;
			if (a->t != b->t)
				return a->t < b->t;
			if (a->f != b->f)
				return a->f < b->f;
			return a->g > b->g;
		}
	};

	Bugsy(int argc, const char *argv[]) :
			SearchAlgorithm<D>(argc, argv),
			timeper(0.0), nresort(0), pertick(20), nexp(0), state(WaitTick),
			closed(30000001) {
		wf = wt = -1;
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-wf") == 0)
				wf = strtod(argv[++i], NULL);
			else if (i < argc - 1 && strcmp(argv[i], "-wt") == 0)
				wt = strtod(argv[++i], NULL);
		}

		if (wf < 0)
			fatal("Must specify non-negative f-weight using -wf");
		if (wt < 0)
			fatal("Must specify non-negative t-weight using -wt");

		nodes = new boost::object_pool<Node>();
	}

	~Bugsy(void) {
		delete nodes;
	}

	enum { WaitTick, ExpandSome, WaitExpand };

	Result<D> &search(D &d, typename D::State &s0) {
		SearchAlgorithm<D>::res.start();
		closed.init(d);
		Node *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		lasttick = walltime();
		while (!open.empty() && !SearchAlgorithm<D>::limit()) {
			updatetime();

			Node* n = *open.pop();
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
		timeper = 0.0;
		state = WaitTick;
		pertick = 20;
		nexp = 0;
		nresort = 0;
		nodes = new boost::object_pool<Node>();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", "binary heap");
		dfpair(stdout, "node size", "%u", sizeof(Node));
		dfpair(stdout, "wt", "%g", wt);
		dfpair(stdout, "wf", "%g", wf);
		dfpair(stdout, "final time per expand", "%g", timeper);
		dfpair(stdout, "number of resorts", "%lu", nresort);
	}

private:

	void expand(D &d, Node *n, State &state) {
		SearchAlgorithm<D>::res.expd++;

		for (unsigned int i = 0; i < d.nops(state); i++) {
			Oper op = d.nthop(state, i);
			if (op == n->pop)
				continue;
			Node *k = kid(d, n, state, op);
			SearchAlgorithm<D>::res.gend++;

			considerkid(d, k);
		}
	}

	void considerkid(D &d, Node *k) {
		unsigned long h = k->packed.hash();
		Node *dup = static_cast<Node*>(closed.find(k->packed, h));
		if (dup) {
			SearchAlgorithm<D>::res.dups++;
			if (k->g >= dup->g) {
				nodes->destroy(k);
				return;
			}

			SearchAlgorithm<D>::res.reopnd++;
			dup->f = dup->f - dup->g + k->g;
			dup->update(*k);
			computeutil(dup);

			if (dup->openind < 0)
				open.push(dup);
			else
				open.update(dup->openind);
			nodes->destroy(k);
		} else {
			closed.add(k, h);
			open.push(k);
		}
	}

	Node *kid(D &d, Node *pnode, State &pstate, Oper op) {
		Node *kid = nodes->construct();
		kid->op = op;
		kid->pop = d.revop(pstate, op);
		kid->parent = pnode;
		Undo u(pstate, op);
		State buf, &kidst = d.apply(buf, pstate, kid->g, op);
		kid->g += pnode->g;
		kid->d = d.d(kidst);
		kid->h = d.h(kidst);
		kid->f = kid->g + kid->h;
		computeutil(kid);
		d.pack(kid->packed, kidst);
		d.undo(pstate, u);
		return kid;
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = 0;
		n0->h = n0->f = d.h(s0);
		n0->d = d.d(s0);
		computeutil(n0);
		n0->op = n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	void computeutil(Node *n) {
		n->t = timeper * n->d;
		n->u = -(wf * n->f + wt * n->t);
	}

	void updatetime(void) {
		double now;
		nexp++;

		switch (state) {
		case WaitTick:
			now = walltime();
			if (now <= lasttick)
				break;
			starttime = now;
			state = ExpandSome;
			break;

		case ExpandSome:
			if (nexp < pertick)
				break;
			lasttick = walltime();
			state = WaitExpand;
			break;

		case WaitExpand:
			now = walltime();
			if (now <= lasttick)
				break;
			updateopen();
			timeper = (now - starttime) / nexp;
			// 1.8 * nexp from Wheeler's bugsy_old.ml
			pertick = nexp * 9 / 5;
			nexp = 0;
			starttime = now;
			state = ExpandSome;
			break;

		default:
			fatal("Unknown update time state: %d\n", state);
		}
	}

	void updateopen(void) {
		nresort++;
		for (unsigned int i = 0; i < open.size(); i++)
			computeutil(open.at(i));
		open.reinit();
	}

	double wf, wt;

	// for nodes-per-second estimation
	double timeper;
	unsigned long nresort, pertick, nexp;
	double starttime, lasttick;
	int state;

	BinHeap<Node, Node*> open;
 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed;
	boost::object_pool<Node> *nodes;
};