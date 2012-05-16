#include "search.hpp"
#include "anyprof.hpp"
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

	~Arastar() {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {
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

			if (wt <= 1.0)
				break;
			nextwt();
			updateopen();
			closed.clear();

		} while(!this->limit() && !open.empty());

		this->finish();
	}

	virtual void reset() {
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

protected:

	// nextwt decrements the weight by the given value.
	void nextwt() {
		wt = wt - dwt > 1.0 ? wt - dwt : 1.0;
		if (wt < 1.0 + sqrt(std::numeric_limits<double>::epsilon()))
			wt = 1.0;
	}

	// rowhdr outputs the incumbent solution row header line.
	void rowhdr() {
		dfrowhdr(stdout, "sol", 7, "num", "nodes expanded",
			"nodes generated", "weight", "solution bound", "solution cost",
			"wall time");
	}

	// row outputs an incumbent solution row.
	void row(unsigned long n, double epsprime) {
		dfrow(stdout, "sol", "uuugggg", n, this->res.expd,
			this->res.gend, wt, epsprime, (double) this->cost,
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
			this->res.reopnd++;
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

	ArastarMon(int argc, const char *argv[]) :
			Arastar<D>(argc, argv), stopnext(false),
			stop(false), nextmon(0) {
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

		AnyProf prof;
		FILE *f = fopen(path.c_str(), "r");
		if (!f)
			fatalx(errno, "failed to open %s for reading", path.c_str());
		prof.read(f);
		fclose(f);
		monitor = MonPolicy(prof, wcost, wtime);

		this->wt = this->wt0;
		this->nodes = new Pool<Node>();
	}

	void search(D &d, typename D::State &s0) {
		this->rowhdr();
		// Output this early for debugging!
		monitor.output(stdout);
		this->start();
		this->closed.init(d);
		this->incons.init(d);
		Node *n0 = init(d, s0);
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

			if (this->wt <= 1.0 || stop)
				break;
			this->nextwt();
			this->updateopen();
			this->closed.clear();

		} while(!stop && !this->limit() && !this->open.empty());

		this->finish();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		Arastar<D>::output(out);
		monitor.output(stdout);
		dfpair(stdout, "cost weight", "%g", wcost);
		dfpair(stdout, "time weight", "%g", wtime);
	}

private:

	bool improve(D &d) {
		bool goal = false;
		mon();
		while (!stop && !this->limit() && this->goodnodes()) {
			Node *n = *this->open.pop();
			State buf, &state = d.unpack(buf, n->packed);

			if (d.isgoal(state)) {
				this->res.goal(d, n);
				goal = true;
			}

			this->expand(d, n, state);
			mon();
		}
		return goal;
	}

	// TODO: don't compute walltime on each call to
	// mon.  Try to learn a good frequency at which to
	// re-check the time.
	void mon() {
		double wallt = walltime();
		double t = wallt - this->res.wallstrt;
		if (wallt < nextmon || this->cost  < 0)
			return;
		if (stopnext) {
			stop = true;
			return;
		}

		std::pair<double, bool> m = monitor.next(this->cost, t);
		fflush(stdout);
		nextmon = wallt + m.first;
		stopnext = m.second;
		if (m.first == 0)
			stop = stopnext;
	}

	double wcost, wtime;
	MonPolicy monitor;
	bool stopnext, stop;
	double nextmon;
};
