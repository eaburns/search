#pragma once
#include "../search/search.hpp"
#include "../utils/geom2d.hpp"
#include "../utils/pool.hpp"
#include <vector>

template <class D>
class Mrastar : public SearchAlgorithm<D> {
private:

	typedef typename D::PackedState PackedState;
	typedef typename D::Operators Operators;
	typedef typename D::State State;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef typename D::Edge Edge;

	class Graph {
	public:

		Graph(SearchAlgorithm<D> &s, unsigned int sz) : nodes(sz), search(s) {
		}

		struct Node;

		struct Out {
			Out(Node *n, double c, Oper o) : node(n), cost(c), op(o) {
			}

			Node *node;
			double cost;
			Oper op;
		};

		struct In {
			In(Node *n, double c) : node(n), cost(c) {
			}

			Node *node;
			double cost;
		};

		struct Node {
			double h;
			// D is not the standard d definition, it is the distance-to-goal estimate
			// of the node from which h was inherited.  It is the number of steps over
			// which heuristic error was acumulated.
			double d;
			// Dorig is the domain's original d estimate for this node.
			double dorig;
			bool isgoal;
			bool expd;	// Have we generated successors yet?
			std::vector<Out> succs;
			std::vector<In> preds;
			PackedState state;

		private:

			friend class Pool<Node>;
			friend class Graph;

			Node() : expd(false) {
			}

			ClosedEntry<Node, D> closedEnt;
		};

		static PackedState &key(Node *n) {
			return n->state;
		}

		static ClosedEntry<Node, D> &closedentry(Node *n) {
			return n->closedEnt;
		}

		// Node returns the node representing this state.
		Node *node(D &d, State &state) {
			Node *n = pool.construct();	
			d.pack(n->state, state);

			unsigned long hash = n->state.hash(&d);
			Node *found = nodes.find(n->state, hash);

			if (found) {
				pool.destruct(n);
				return found;
			}

			n->h = d.h(state);
			n->d = n->dorig = d.d(state);
			n->isgoal = d.isgoal(state);
			nodes.add(n, hash);
			return n;
		}

		// Succs returns the successors of the given node, either by
		// returning their cached values or by generating them.
		std::vector<Out>&succs(D &d, Node *n) {
			if (n->expd)
				return n->succs;

			search.res.expd++;

			State buf, &s = d.unpack(buf, n->state);
			Operators ops(d, s);
			for (unsigned int i = 0; i < ops.size(); i++) {
				search.res.gend++;

				Edge e(d, s, ops[i]);
				Node *k = node(d, e.state);
				k->preds.emplace_back(n, e.cost);
				n->succs.emplace_back(k, e.cost, ops[i]);
			}

			n->expd = true;
			return n->succs;
		}

		void clear() {
			pool.releaseall();
			nodes.clear();
		}

	private:
		Pool<Node> pool;
		ClosedList<Graph, Node, D> nodes;

		// The search algorithm for counting expansions, generations, duplicates, etc.
		SearchAlgorithm<D> &search;
	};

	// Lss is a resumable A* search, defining the local search space beneath a node.
	class Lss {
	public:

		typedef typename Graph::Node GraphNode;

		struct Node {
			long openind, learnind;
			double g, f, fhat;
			unsigned long gend;
			Oper op;
			Node *parent;
			bool closed, updated;
			GraphNode *node;

		private:

			friend class Lss;
			friend class Pool<Node>;

			Node() : openind(-1), learnind(-1), closed(false), updated(false) {
			}

			ClosedEntry<Node, D> closedEnt;
		};

		static PackedState &key(Node *n) {
			return n->node->state;
		}

		static ClosedEntry<Node, D> &closedentry(Node *n) {
			return n->closedEnt;
		}

		// F orders nodes in increasing order of f, tie-breaking on high g.
		class F {
		public:
			static void setind(Node *n, long i) {
				n->openind = i;
			}

			static bool pred(Node *a, Node *b) {
				if (a->fhat == b->fhat) {
					if (a->f == b->f)
						return a->g > b->g;
					return a->f < b->f;
				}
				return a->fhat < b->fhat;
			}
		};

		Lss(Mrastar<D> &s, Graph &g, GraphNode *c, GraphNode *rt, double g0, Oper o) :
			goal(NULL), op(o), cur(c), nodes(s.lookahead), nclosed(0), search(s), graph(g) {

			root = pool.construct();
			root->node = rt;
			root->g = g0;
			root->f = rt->h;
			root->gend = search.ndelay;
			assert(search.herr >= 0);
			root->fhat = root->f + search.herr*rt->d;
			root->parent = NULL;
			root->op = op;
			if (rt->isgoal)
				goal = root;
			open.push(root);
		}

		// Expand performs no more than N expansions, returning true
		// if there are more nodes to be expanded and false otherwise.
		// If a goal is expanded then expand returns early.  If a goal was
		// expanded on a previous call then it return immediately.
		bool expand(D &d, unsigned int N) {
			if (open.empty() || (goal && goal->closed))
				return !open.empty();

			unsigned int expd = 0;
			while (!open.empty() && expd < N && !search.limit()) {
				Node *n = *open.pop();

				nclosed += !n->closed;
				n->closed = true;

				if (n->node->isgoal) {
					assert (goal);
					assert (goal == n || geom2d::doubleeq(goal->g, n->g));
					break;
				}

				search.updatedelay(n->gend);

				expd++;
				Node *bestkid = NULL;
				for (auto e : graph.succs(d, n->node)) {
					if (e.node == cur || (n->parent && e.node == n->parent->node))
						continue;

					unsigned long hash = e.node->state.hash(&d);
					Node *k = nodes.find(e.node->state, hash);
					double g = n->g + e.cost;

					if (!k) {
						k = pool.construct();
						k->node = e.node;
						k->gend = search.ndelay;
						nodes.add(k, hash);
					} else if (k->g <= g) {
						if (!bestkid || k->f < bestkid->f)
							bestkid = k;
						continue;
					}

					k->parent = n;
					k->op = e.op;
					k->g = g;
					k->f = g + k->node->h;
					assert (search.herr >= 0);
					k->fhat = k->f + search.herr*k->node->d;
					open.pushupdate(k, k->openind);

					if (!bestkid || k->f < bestkid->f)
						bestkid = k;

					if (k->node->isgoal && (!goal || k->g < goal->g))
						goal = k;
				}

				if (bestkid)
					search.updateerror(n->node, n->g - bestkid->g, bestkid->node);
			}

			return !open.empty();
		}

		// H orders nodes in increasing order on h.
		class H {
		public:
			static void setind(Node *n, long i) {
				n->learnind = i;
			}

			static bool pred(Node *a, Node *b) {
				return a->node->h < b->node->h;
			}
		};

		// Learn backs up the h values on the fringe into the interior.
		void learn() {
			BinHeap<H, Node*> o;

			for (auto n : nodes) {
				n->learnind = -1;
				n->updated = false;
			}

			o.append(open.data());

			while (nclosed > 0 && !o.empty()) {
				Node *n = *o.pop();

				if (n->closed);
					nclosed--;
				n->closed = false;

				for (auto e : n->node->preds) {
					Node *p = nodes.find(e.node->state);
					if (!p || !p->closed)
						continue;
					if (!p->updated || p->node->h > n->node->h + e.cost) {
						p->node->h = n->node->h + e.cost;
						if (p->node->d < n->node->d)
							p->node->d = n->node->d;
						p->updated = true;
						o.pushupdate(p, p->learnind);
					}
				}
			}
		}

		// Move returns the operators (in reverse order) from the root
		// to the cheapest generated goal, or, if no goal was generated,
		// the node on the open list with the lowest f value, along with
		// the corresponding graph node.
		std::pair<std::vector<Oper>, GraphNode*> move(D &d) {
			Node *best = goal;

			if (!best) {
				assert (!open.empty());
				best = open.data()[0];
				for (auto n : open.data())
					assert (!F::pred(n, best));
			}

			std::vector<Oper> ops;
			for (Node *n = best; n; n = n->parent)
				ops.push_back(n->op);


			return std::make_pair(ops, best->node);
		}

		static void setind(Lss *l, long i) {
		}

		static bool pred(Lss *a, Lss *b) {
			return a->f() < b->f();
		}

		double f() {
			if (open.empty() && !goal)
				return geom2d::Infinity;
			if (open.empty())
				return goal->g;
			return (*open.front())->fhat;
		}

		// Goal is the cheapest goal that has been generated, or NULL
		// if no goal was generated.  If goal->closed is true then the
		// goal was expanded, and this is the optimal solution from the
		// root of this search.
		Node *goal;

		// Root is the root of this node.
		Node *root;

		// Op is the operator generating the root of this tree from
		// the current node.
		Oper op;

	private:

		GraphNode *cur;

		Pool<Node> pool;
		BinHeap<F, Node*> open;
		ClosedList<Lss, Node, D> nodes;
		unsigned int nclosed;

		// The search algorithm used to check the limit.
		Mrastar<D> &search;
		Graph &graph;
	};

public:

	Mrastar(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv),
		herr(0), ndelay(0),  herrNext(0), nerr(0),
		delay(1),
		nsearch(0),
		graph(*this, 30000001),
		lookahead(0) {

		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-lookahead") == 0)
				lookahead = strtoul(argv[++i], NULL, 10);
		}
		if (lookahead < 1)
			fatal("Must specify a lookahead ≥1 using -lookahead");
	}

	~Mrastar() {
	}

	typedef typename Graph::Node Node;

	void search(D &d, State &s0) {
		this->start();

		Node *cur = graph.node(d, s0);

		while (!cur->isgoal && !this->limit()) {
			auto p = step(d, cur);

			for (auto s : cur->succs) {
				if (s.cost + s.node->h > cur->h)
					cur->h = s.cost + s.node->h;
			}

			cur = p.second;
			this->res.ops.insert(this->res.ops.end(), p.first.rbegin(), p.first.rend());
			steps.emplace_back(cputime() - this->res.cpustart, (unsigned int) p.first.size());
		}
		this->finish();

		if (!cur->isgoal) {
			this->res.ops.clear();
			return;
		}

		// Rebuild the path from the operators.
		graph.clear();
		this->res.path.push_back(s0);
		for (auto o : this->res.ops) {
			State copy = this->res.path.back();
			Edge e(d, copy, o);
			this->res.path.push_back(e.state);
		}

		PackedState pkd;
		d.pack(pkd, this->res.path.back());
		assert (pkd.eq(&d, cur->state));

		assert (d.isgoal(this->res.path.back()));
		std::reverse(this->res.ops.begin(), this->res.ops.end());
		std::reverse(this->res.path.begin(), this->res.path.end());
	}

	std::pair<std::vector<Oper>, Node*> step(D &d, Node *cur) {
		BinHeap<Lss, Lss*> lss;
		for (auto s : graph.succs(d, cur))
			lss.push(new Lss(*this, graph, cur, s.node, s.cost, s.op));

		nexterror();

 		while (!this->limit()) {

			if ((*lss.front())->goal)
				break;

			Lss *l = searchhere(lss);
			if (!l) {
//fprintf(stderr, "move\n");
				break;
			}

			double start = cputime();

			l->expand(d, lookahead);

			if (l->goal) {
				auto p = move(d, l);
				for (auto l : lss.data())
					delete l;
				return p;
			}

			nsearch++;
			searchtime += ((cputime()-start) - searchtime)/nsearch;
		}

		auto p = move(d, *lss.front());

		for (auto l : lss.data())
			delete l;

		return p;
	}

	// Searchhere returns the Lss from which to perform the next lookahead
	// expansions or NULL if it's time to act.
	Lss *searchhere(BinHeap<Lss, Lss*> &lss) {

		if (this->res.expd == 1 && lss.size() > 0)
			return *lss.front();

		// Only one action: Nike.
		if (lss.size() == 1)
			return NULL;

		for (auto l : lss.data())
			l->learn();

		Lss *best = NULL;
		double ubest = (*lss.front())->root->fhat;
//fprintf(stderr, "uact=%g\n", ubest);

		Lss *first = *lss.pop();	// We'll put this one back later.
		Lss *second = *lss.front();
		double u = expectedutil(second->root->fhat, first);
		if (u < ubest) {
			best = first;
			ubest = u;
		}

//int i = 0;
//fprintf(stderr, ", u%d=%g\n", i, u);

		for (auto l : lss.data()) {
			double u = expectedutil(first->root->fhat, l);
//i++;
//fprintf(stderr, ", u%d=%g\n", i, u);
			if (u < ubest) {
				best = l;
				ubest = u;
			}
		}
//fprintf(stderr, "nsearch=%lu\n", nsearch);

		lss.push(first);

		return best;
	}

	double expectedutil(double fbest, Lss *l) {
		double u = geom2d::Infinity;
		for (unsigned int i = 1; i < 10000; i++) {
			auto e = util(fbest, l, lookahead*i);
			if (e.first < u)
				 u = e.first;
			if (e.second)
				break;
		}
		return u;
	}

	// Util is the expected utility of searching under the given LSS for  the given amount of search.
	std::pair<double, bool> util(double fbest, Lss *l, unsigned long N) {
		double dhat = l->root->node->d;
		double dfrac = (N/delay) / dhat;
		if (dfrac > 1)
			dfrac = 1;
		else if (dfrac < 0)
			dfrac = 0;

//		double sigma = herr * dhat*(1 - dfrac);
		dhat *= 1 - dfrac;
		double sigma = 12.698373624401009
			+ dhat * 0.034835695393688
			+ dhat*dhat * 0.000034990580392
			+ dhat*dhat*dhat * -0.000000038671449;

		Normal n(l->root->fhat, sigma);

		double min = l->root->fhat - 3*sigma;
		double max = fbest;

		double searchcost = (lookahead/N)*(searchtime/0.02);	// 0.02 is the number of seconds per frame in plat2d ☹

//fprintf(stderr, "max=%g, min=%g, f=%g, fhat=%g, sigma=%g, dhat=%g, dfrac=%g, herr=%g, delay=%g, nsteps=%lu\n", max, min, l->root->f, l->root->fhat, sigma, dhat, dfrac, herr, delay, (unsigned long) steps.size());
		if (max <= min)
			return std::make_pair(searchcost + fbest, geom2d::doubleeq(dfrac, 1));

		if (geom2d::doubleeq(sigma, 0))
			return std::make_pair(searchcost + l->root->fhat, geom2d::doubleeq(dfrac, 1));

		return std::make_pair(
				searchcost +
				integrate([n](double x) -> double{ return x * n.pdf(x); }, min, max, 0.1) + 
				(1-n.cdf(fbest))*fbest,
			geom2d::doubleeq(dfrac, 1));
	}

	void nexterror() {
		if (herrNext > 0)
			herr = herrNext;
		herrNext = 0;
		nerr = 0;
	}

	void updateerror(Node *n, double cost, Node *bestkid) {
		nerr++;

		double h = bestkid->h;
		if (h < n->h - cost)
			h = n->h - cost;

		double e = (h + cost) - n->h;
		if (e < 0)
			e = 0;
		herrNext += (e - herrNext)/nerr;
	}

	void updatedelay(unsigned long gend) {
		unsigned long d = ndelay - gend;
		ndelay++;
		delay += (d - delay)/(double)ndelay;
	}

	std::pair<std::vector<Oper>, Node*> move(D &d, Lss *l) {
		std::vector<Oper> ops;
		ops.push_back(l->op);
		return std::make_pair(ops, l->root->node);
	}

	void reset() {
		SearchAlgorithm<D>::reset();
		steps.clear();
		graph.clear();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		dfpair(out, "num steps", "%lu", (unsigned long) steps.size());
		if (steps.size() != 0) {
			double mint = steps.front().time;
			double maxt = steps.front().time;

			unsigned int minl = steps.front().length;
			unsigned int maxl = steps.front().length;
			unsigned long nmoves = steps.front().length;

			for (unsigned int i = 1; i < steps.size(); i++) {
				double dt = steps[i].time - steps[i-1].time;
				if (dt < mint)
					mint = dt;
				if (dt > maxt)
					maxt = dt;

				unsigned int l = steps[i].length;
				if (l < minl)
					minl = l;
				if (l > maxl)
					maxl = l;
				nmoves += l;
			}
			dfpair(out, "first emit cpu time", "%f", steps.front().time);
			dfpair(out, "min step cpu time", "%f", mint);
			dfpair(out, "max step cpu time", "%f", maxt);
			dfpair(out, "mean step cpu time", "%f", (steps.back().time-steps.front().time)/steps.size());
			dfpair(out, "min step length", "%u", minl);
			dfpair(out, "max step length", "%u", maxl);
			dfpair(out, "mean step length", "%g", nmoves / (double) steps.size());
			dfpair(out, "number of searches", "%lu", nsearch);
		}

		dfpair(out, "last step mean h error", "%g", herr);
	}

	// Herr is the previous iteration's average heuristic error.
	double herr;

	// Ndelay is the number of expansion over which the delay is averaged.
	unsigned long ndelay;

private:

	struct Step {
		Step(double t, unsigned int l) : time(t), length(l) {
		}

		double time;
		unsigned int length;
	};

	std::vector<Step> steps;

	// HerrNext is the current iteration's average heuristic error.
	double herrNext;

	// NErr is the number of items over which error has been computed this iteration.
	unsigned long nerr;

	// Delay is the average expansion delay.
	double delay;

	// Searchtime is the average time to perform lookahead amount of search.
	double searchtime;

	// Nsearch is the number of searches over which the search time is averaged;
	unsigned long nsearch;

	Graph graph;
	unsigned int lookahead;
};
