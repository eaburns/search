#include "search.hpp"
#include "anyprof/profile.hpp"
#include "../structs/binheap.hpp"
#include "../utils/pool.hpp"
#include <limits>
#include <vector>
#include <cmath>
#include <cerrno>
#include <string>

void dfrowhdr(FILE*, const char*, unsigned int ncols, ...);
void dfrow(FILE*, const char*, const char*, ...);
void fatal(const char*, ...);
void fatalx(int, const char*, ...);

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
	
		std::vector<Node*> &nodes() { return incons; }
	
		void clear() {
			mem.clear();
			incons.clear();
		}
	
	private:
	 	ClosedList<SearchNode<D>, SearchNode<D>, D> mem;
		std::vector<Node*> incons;
	};

	Arastar(int argc, const char *argv[]) :
			SearchAlgorithm<D>(argc, argv), closed(30000001),
			incons(30000001), cost(-1) {

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

	~Arastar() {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {
		bool optimal = false;
		rowhdr();
		this->start();
		closed.init(d);
		incons.init(d);

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
				row(n, epsprime);
			}

			if (wt <= 1.0) {
				optimal = true;
				break;
			}
			nextwt();
			updateopen();
			closed.clear();

		} while(!this->limit() && !open.empty());

		this->finish();
		dfpair(stdout, "converged", "%s", optimal ? "yes" : "no");
	}

	virtual void reset() {
		SearchAlgorithm<D>::reset();
		wt = wt0;
		open.clear();
		closed.clear();
		incons.clear();
		delete nodes;
		nodes = new Pool<Node>();
		cost = Cost(-1);
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", "binary heap");
		dfpair(stdout, "node size", "%u", sizeof(Node));
		dfpair(stdout, "initial weight", "%g", wt0);
		dfpair(stdout, "weight decrement", "%g", dwt);
	}

protected:

	// nextwt decrements the weight by the given value.
	void nextwt() {
		wt = wt - dwt > 1.0 ? wt - dwt : 1.0;
		if (wt < 1.0 + sqrt(std::numeric_limits<double>::epsilon()))
			wt = 1.0;
	}

	// rowhdr outputs the incumbent solution row header line.
	void rowhdr() {
		dfrowhdr(stdout, "incumbent", 7, "num", "nodes expanded",
			"nodes generated", "weight", "solution bound", "solution cost",
			"wall time");
	}

	// row outputs an incumbent solution row.
	void row(unsigned long n, double epsprime) {
		dfrow(stdout, "incumbent", "uuugggg", n, this->res.expd,
			this->res.gend, wt, epsprime, cost,
			walltime() - this->res.wallstrt);
	}

	bool improve(D &d) {
		bool goal = false;
		while (!this->limit() && goodnodes()) {
			Node *n = *open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state)) {
				cost = (double) n->g;
				this->res.goal(d, n);
				goal = true;
			}

			expand(d, n, state);
		}
		return goal;
	}

	bool goodnodes() {
		return !open.empty() && (cost == Cost(-1) || cost > (*open.front())->fprime);
	}

	// findbound finds and returns the tightest bound for
	// the current incumbent.
	double findbound() {
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

	// updateopen updates the f' values of nodes in incons and
	// on the open list, then incons is added to the open list.
	void updateopen() {
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

		typename D::Operators ops(d, state);
		for (unsigned int i = 0; i < ops.size(); i++) {
			if (ops[i] == n->pop)
				continue;

			this->res.gend++;
			considerkid(d, n, state, ops[i]);
		}
	}

	void considerkid(D &d, Node *parent, State &state, Oper op) {
		Node *kid = nodes->construct();
		typename D::Edge e(d, state, op);
		kid->g = parent->g + e.cost;
		d.pack(kid->packed, e.state);

		unsigned long hash = d.hash(kid->packed);
		Node *dup = static_cast<Node*>(closed.find(kid->packed, hash));
		if (dup) {
			this->res.dups++;
			if (kid->g >= dup->g) {
				nodes->destruct(kid);
				return;
			}
			dup->fprime = dup->fprime - dup->g + kid->g;
			dup->update(kid->g, parent, op, e.revop);
			if (dup->ind < 0)
				incons.add(dup, hash);
			else
				open.update(dup->ind);
			nodes->destruct(kid);
		} else {
			kid->h = d.h(e.state);
			kid->fprime = (double) kid->g + wt * kid->h;
			kid->update(kid->g, parent, op, e.revop);
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

	double cost;	// solution cost
};


// ArastarMon is an implementation of ARA* that uses a
// monitor in order to stop it when the utility of the current
// solution/search time is estimated to be better than that
// which is achievable from further search.
template <class D> struct ArastarMon : public Arastar<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef typename Arastar<D>::Node Node;
	typedef typename Arastar<D>::Incons Incons;

	ArastarMon(int argc, const char *argv[]) : Arastar<D>(argc, argv) {
		this->wt0 = this->dwt = -1;
		wcost = wtime = 1;
		std::string path;
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-wt0") == 0)
				this->wt0 = strtod(argv[++i], NULL);

			else if (i < argc - 1 && strcmp(argv[i], "-dwt") == 0)
				this->dwt = strtod(argv[++i], NULL);

			else if (i < argc -1 && strcmp(argv[i], "-wf") == 0)
				wcost = strtod(argv[++i], NULL);

			else if (i < argc - 1 && strcmp(argv[i], "-wt") == 0)
				wtime = strtod(argv[++i], NULL);

			else if (i < argc - 1 && strcmp(argv[i], "-p") == 0)
				path = argv[++i];
		}

		if (this->wt0 < 1)
			fatal("Must specify initial weight ≥ 1 using -wt0");
		if (this->dwt <= 0)
			fatal("Must specify weight decrement > 0 using -dwt");
		if (path == "")
			fatal("Must specify a profile file");

		mon = AnytimeMonitor(AnytimeProfile(path), wcost, wtime);

		this->wt = this->wt0;
		this->nodes = new Pool<Node>();
	}

	void search(D &d, typename D::State &s0) {
 		bool optimal = false;
		this->rowhdr();
		this->start();
		this->closed.init(d);
		this->incons.init(d);
		Node *n0 = this->init(d, s0);
		this->closed.add(n0);
		this->open.push(n0);

		unsigned long n = 0;
		do {
			if (improve(d)) {
				n++;
				double epsprime = this->wt == 1.0 ? 1.0 : this->findbound();
				if (this->wt < epsprime)
					epsprime = this->wt;
				this->row(n, epsprime);
			}
			if (this->wt <= 1.0)
				optimal = true;
			if (this->wt <= 1.0 || shouldstop())
				break;
			this->nextwt();
			this->updateopen();
			this->closed.clear();

		} while(!shouldstop() && !this->limit() && !this->open.empty());

		this->finish();
		dfpair(stdout, "converged", "%s", optimal ? "yes" : "no");
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		Arastar<D>::output(out);
		dfpair(stdout, "wf", "%g", wcost);
		dfpair(stdout, "wt", "%g", wtime);
	}

private:

	bool improve(D &d) {
		bool goal = false;
		while (!shouldstop() && !this->limit() && this->goodnodes()) {
			Node *n = *this->open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state)) {
				this->cost = (double) n->g;
				this->res.goal(d, n);
				goal = true;
			}

			this->expand(d, n, state);
		}
		return goal;
	}

	bool shouldstop() const {
		if (this->cost < 0)
			return false;
		double t = walltime() - this->res.wallstrt;
		double c = this->cost;
		return mon.stop(c, t);
	}

	double wcost, wtime;
	AnytimeMonitor mon;
};
