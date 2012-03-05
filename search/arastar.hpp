#include "search.hpp"
#include "anyprof.hpp"
#include "../structs/binheap.hpp"
#include "../utils/pool.hpp"
#include <limits>
#include <vector>
#include <cmath>
#include <cerrno>

void dfrowhdr(FILE *, const char *name, int ncols, ...);
void dfrow(FILE *, const char *name, const char *colfmt, ...);
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

	virtual ~Arastar(void) {
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

protected:

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
		typename D::Transition tr(d, state, op);
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
			if (dup->openind < 0)
				incons.add(dup, hash);
			else
				open.update(dup->openind);
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
		this->start();
		this->closed.init(d);
		this->incons.init(d);

		dfrowhdr(stdout, "sol", 7, "num", "nodes expanded",
			"nodes generated", "weight", "solution bound", "solution cost",
			"wall time");

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
	
				dfrow(stdout, "sol", "uuugggg", n, this->res.expd,
					this->res.gend, this->wt, epsprime,
					(double) this->res.cost, walltime() - this->res.wallstrt);
			}

			if (this->wt <= 1.0 || stop)
				break;

			this->wt = this->wt - this->dwt > 1.0 ? this->wt - this->dwt : 1.0;
			if (this->wt < 1.0 + sqrt(std::numeric_limits<double>::epsilon()))
				this->wt = 1.0;

			this->updateopen();
			this->closed.clear();

		} while(!stop && !this->limit() && !this->open.empty());

		this->finish();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		Arastar<D>::output(out);
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
	void mon(void) {
		double wallt = walltime();
		double t = wallt - this->res.wallstrt;
		if (wallt < nextmon || this->res.cost  < 0)
			return;
		if (stopnext) {
			printf("stopping\n");
			stop = true;
			return;
		}

		std::pair<double, bool> m = monitor.next(this->res.cost, t);
		printf("Δt=%g, stop=%d\n", m.first, m.second);
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
