#include "../search/search.hpp"
#include "../structs/binheap.hpp"
#include "../utils/pool.hpp"
#include <limits>
#include <vector>
#include <cmath>

void dfrowhdr(FILE*, const char*, unsigned int ncols, ...);
void dfrow(FILE*, const char*, const char*, ...);
void fatal(const char*, ...);

template <class D> struct Arastar : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node : SearchNode<D> {
		Cost h;
		double fprime;
	
		static bool pred(Node *a, Node *b) {
			if (a->fprime == b->fprime)
				return a->g > b->g;
			return a->fprime < b->fprime;
		}
	};
	
	struct Incons {
		Incons(unsigned long szhint) : mem(szhint) { }
	
		void init(D &d) { mem.init(d); }
	
		void add(Node *n, unsigned long h) {
			if (mem.find(n->packed, h) != NULL)
				return;
			mem.add(n);
			incons.push_back(n);
		}
	
		std::vector<Node*> &nodes(void) { return incons; }
	
		void clear(void) {
			mem.clear();
			incons.clear();
		}
	
	private:
	 	ClosedList<SearchNode<D>, SearchNode<D>, D> mem;
		std::vector<Node*> incons;
	};

	Arastar(int argc, const char *argv[]) :
			SearchAlgorithm<D>(argc, argv), closed(30000001),
			incons(30000001) {

		wt0 = dwt = -1;
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-wt0") == 0)
				wt0 = strtod(argv[++i], NULL);
			else if (i < argc - 1 && strcmp(argv[i], "-dwt") == 0)
				dwt = strtod(argv[++i], NULL);
		}

		if (wt0 < 1)
			fatal("Must specify initial weight ≥ 1 using -wt0");
		if (dwt <= 0)
			fatal("Must specify weight decrement > 0 using -dwt");

		wt = wt0;
		nodes = new Pool<Node>();
	}

	~Arastar(void) {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {
		this->start();
		closed.init(d);
		incons.init(d);

		dfrowhdr(stdout, "sol", 7, "num", "nodes expanded",
			"nodes generated", "weight", "solution bound", "solution cost",
			"wall time");


		Node *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		unsigned long n = 0;
		do {
			if (improve(d)) {
				n++;
				double epsprime = wt == 1.0 ? 1.0 : findbound();
				if (wt < epsprime)
					epsprime = wt;
	
				dfrow(stdout, "sol", "uuugggg", n, this->res.expd,
					this->res.gend, wt, epsprime, (double) this->res.cost,
					walltime() - this->res.wallstrt);
			}

			if (wt <= 1.0)
				break;

			wt = wt - dwt > 1.0 ? wt - dwt : 1.0;
			if (wt < 1.0 + sqrt(std::numeric_limits<double>::epsilon()))
				wt = 1.0;

			updateopen();
			closed.clear();

		} while(!this->limit() && !open.empty());

		this->finish();
	}

	virtual void reset(void) {
		SearchAlgorithm<D>::reset();
		wt = wt0;
		open.clear();
		closed.clear();
		incons.clear();
		delete nodes;
		nodes = new Pool<Node>();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", "binary heap");
		dfpair(stdout, "node size", "%u", sizeof(Node));
		dfpair(stdout, "initial weight", "%g", wt0);
		dfpair(stdout, "weight decrement", "%g", dwt);
	}

private:

	bool improve(D &d) {
		bool goal = false;
		while (!this->limit() && goodnodes()) {
			Node *n = *open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state)) {
				this->res.goal(d, n);
				goal = true;
			}

			expand(d, n, state);
		}
		return goal;
	}

	bool goodnodes() {
		return !open.empty() &&
			(this->res.cost == Cost(-1) ||
			(double) this->res.cost > (*open.front())->fprime);
	}

	// Find the tightest bound for the current incumbent.
	double findbound(void) {
		assert (this->res.cost != Cost(-1));
		double cost = this->res.cost;
		double min = std::numeric_limits<double>::infinity();

		std::vector<Node*> &inodes = incons.nodes();
		for (long i = 0; i < (long) inodes.size(); i++) {
			Node *n = inodes[i];
			double f = n->g + n->h;
			if (f >= cost) {
				inodes[i] = inodes[inodes.size()-1];
				inodes.pop_back();
				i--;
				continue;
			}
			if (f < min)
				min = f;
		}

		std::vector<Node*> &onodes = open.data();
		for (long i = 0; i < (long) onodes.size(); i++) {
			Node *n = onodes[i];
			double f = n->g + n->h;
			if (f >= cost) {
				onodes[i] = onodes[onodes.size()-1];
				onodes.pop_back();
				i--;
				continue;
			}
			if (f < min)
				min = f;
		}
		return cost / min;
	}

	// Update the open list: update f' values and add INCONS
	// and re-heapify.
	void updateopen(void) {
		std::vector<Node*> &nodes = incons.nodes();
		for (unsigned long i = 0; i < nodes.size(); i++) {
			Node *n = nodes[i];
			n->fprime = (double) n->g + wt * n->h;
		}
		for (long i = 0; i < open.size(); i++) {
			Node *n = open.at(i);
			n->fprime = (double) n->g + wt * n->h;
		}
		open.append(incons.nodes());	// reinits heap property
		incons.clear();
	}

	void expand(D &d, Node *n, State &state) {
		this->res.expd++;
		for (unsigned int i = 0; i < d.nops(state); i++) {
			Oper op = d.nthop(state, i);
			if (op == n->pop)
				continue;

			this->res.gend++;
			considerkid(d, n, state, op);
		}
	}

	void considerkid(D &d, Node *parent, State &state, Oper op) {
		Node *kid = nodes->construct();
		typename D::Edge tr(d, state, op);
		kid->g = parent->g + tr.cost;
		d.pack(kid->packed, tr.state);

		unsigned long hash = kid->packed.hash();
		Node *dup = static_cast<Node*>(closed.find(kid->packed, hash));
		if (dup) {
			this->res.dups++;
			if (kid->g >= dup->g) {
				nodes->destruct(kid);
				return;
			}
			this->res.reopnd++;
			dup->fprime = dup->fprime - dup->g + kid->g;
			dup->update(kid->g, parent, op, tr.revop);
			if (dup->ind < 0)
				incons.add(dup, hash);
			else
				open.update(dup->ind);
			nodes->destruct(kid);
		} else {
			kid->h = d.h(tr.state);
			kid->fprime = (double) kid->g + wt * kid->h;
			kid->update(kid->g, parent, op, tr.revop);
			closed.add(kid, hash);
			open.push(kid);
		}
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->packed, s0);
		n0->g = Cost(0);
		n0->h = d.h(s0);
		n0->fprime = wt * n0->h;
		n0->op = n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	double wt0, dwt, wt;
	BinHeap<Node, Node*> open;
 	ClosedList<SearchNode<D>, SearchNode<D>, D> closed;
	Incons incons;
	Pool<Node> *nodes;
};
