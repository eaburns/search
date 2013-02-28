// LSS-LRTA* using a depth-first search to generate the LSS.

#pragma once
#include "../search/search.hpp"
#include "../utils/geom2d.hpp"
#include "../utils/pool.hpp"
#include <vector>

template <class D>
class Dflrtastar : public SearchAlgorithm<D> {
private:

	typedef typename D::PackedState PackedState;
	typedef typename D::Operators Operators;
	typedef typename D::State State;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef typename D::Edge Edge;

	class Node;
	class Nodes;

	struct inedge {
		inedge(Node *n, double c) : node(n), incost(c) {
		}

		Node *node;
		double incost;
	};

	struct outedge {
		outedge(Node *n, Oper o, Cost rc, Cost c) :
			node(n),
			op(o),
			revcost(rc == Cost(-1) ? geom2d::Infinity : rc),
			outcost(c == Cost(-1) ? geom2d::Infinity : c) {
		}

		Node *node;
		Oper op;
		double revcost;
		double outcost;
	};

	class Node {
	public:
		static ClosedEntry<Node, D> &closedentry(Node *n) {
			return n->closedent;
		}

		static PackedState &key(Node *n) {
			return n->state;
		}

		PackedState state;
		double h;
		bool dead;
		bool expd;	// was this expanded before?
		bool goal;
		std::vector<outedge> succs;
		std::vector<inedge> preds;

	private:
		friend class Nodes;
		friend class Pool<Node>;

		Node() : dead(false), expd(false) {
		}

		ClosedEntry<Node, D> closedent;
	};

	class Nodes {
	public:
		Nodes(unsigned int sz) : tbl(sz) {
		}

		~Nodes() {
		}

		void clear() {
			tbl.clear();
			pool.releaseall();
		}

		Node *get(D &d, State &s) {
			Node *n = pool.construct();
			d.pack(n->state, s);

			unsigned long hash = n->state.hash(&d);
			Node *found = tbl.find(n->state, hash);
			if (found) {
				pool.destruct(n);
				return found;
			}

			n->goal = d.isgoal(s);
			n->h = d.h(s);
			tbl.add(n, hash);
			return n;
		}

	private:
		ClosedList<Node, Node, D> tbl;
		Pool<Node> pool;
	};

	class LssNode {
	public:

		LssNode() : openind(-1), updated(false), frontier(-1) {
		}
	
		class Nodes {
		public:
			static ClosedEntry<LssNode, D> &closedentry(LssNode *n) {
				return n->nodesent;
			}
	
			static PackedState &key(LssNode *n) {
				return n->node->state;
			}
		};

		class HSort {
		public:
			static void setind(LssNode *n, int i) {
				n->openind = i;
			}
		
			static bool pred(LssNode *a, LssNode *b) {
				return a->node->h < b->node->h;
			}
		};

		Node *node;
		LssNode *parent;
		double g, f;
		Oper op;
		long openind;
		bool updated;
		long frontier;

	private:
		ClosedEntry<LssNode, D> nodesent;
	};

public:

	Dflrtastar(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv),
		lssNodes(1),
		avgexp(0),
		nodes(30000001),
		depthlimit(0) {

		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-depth") == 0)
				depthlimit = strtoul(argv[++i], NULL, 10);
		}
		if (depthlimit < 1)
			fatal("Must specify a lookahead ≥1 using -depth");

		lssNodes.resize(pow(2, depthlimit)*3);
	}

	~Dflrtastar() {
	}

	void reset() {
		SearchAlgorithm<D>::reset();
		nodes.clear();
		avgexp = 0;
		lssFrontier.clear();
		lssNodes.clear();
		lssPool.releaseall();
	}

	void search(D &d, State &s0) {
		this->start();

		Node *cur = nodes.get(d, s0);

		while (!cur->goal && !this->limit()) {
			LssNode *goal = expandLss(d, cur);
			if (!goal && !this->limit())
				hCostLearning(d);
			cur = move(cur, goal);
			times.push_back(cputime() - this->res.cpustart);
		}

		this->finish();

		if (!cur->goal) {
			this->res.ops.clear();
			return;
		}

		// Rebuild the path from the operators.
		nodes.clear();
		this->res.path.push_back(s0);
		for (auto it = this->res.ops.begin(); it != this->res.ops.end(); it++) {
			State copy = this->res.path.back();
			Edge e(d, copy, *it);
			this->res.path.push_back(e.state);
		}
		std::reverse(this->res.ops.begin(), this->res.ops.end());
		std::reverse(this->res.path.begin(), this->res.path.end());
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		dfpair(out, "average lookahead expansions", "%g", avgexp);
		dfpair(out, "num steps", "%lu", (unsigned long) times.size());
		assert (lengths.size() == times.size());
		if (times.size() != 0) {
			double min = times.front();
			double max = times.front();
			for (unsigned int i = 1; i < times.size(); i++) {
				double dt = times[i] - times[i-1];
				if (dt < min)
					min = dt;
				if (dt > max)
					max = dt;
			}
			dfpair(out, "first emit cpu time", "%f", times.front());
			dfpair(out, "min step cpu time", "%f", min);
			dfpair(out, "max step cpu time", "%f", max);
			dfpair(out, "mean step cpu time", "%f", (times.back()-times.front())/times.size());
		}
		if (lengths.size() != 0) {
			unsigned int min = lengths.front();
			unsigned int max = lengths.front();
			unsigned long sum = 0;
			for (auto l : lengths) {
				if (l < min)
					min = l;
				if (l > max)
					max = l;
				sum += l;
			}
			dfpair(out, "min step length", "%u", min);
			dfpair(out, "max step length", "%u", max);
			dfpair(out, "mean step length", "%g", sum / (double) lengths.size());
		}
	}

private:

	LssNode *expandLss(D &d, Node *rootNode) {
		lssNodes.clear();
		lssFrontier.clear();
		lssPool.releaseall();
		nexpd = nnodes = 0;

		LssNode *a = lssPool.construct();
		a->node = rootNode;
		a->parent = NULL;
		a->op = D::Nop;
		a->g = 0;
		a->f = rootNode->h;
		lssNodes.add(a);

		alpha = geom2d::Infinity;

		dfs(d, 0, a);

		avgexp += (nexpd - avgexp) / (times.size()+1);

		return NULL;
	}

	void dfs(D &d, unsigned int depth, LssNode *n) {
		if (this->limit() || depth > depthlimit || alpha <= n->f || n->node->goal) {
			alpha = std::min(alpha, n->f);
			if (n->frontier < 0) {
				n->frontier = lssFrontier.size();
				lssFrontier.push_back(n);
			}
			return;
		}

		nexpd++;
		for (auto e : expand(d, n->node)) {
			Node *k = e.node;
			if (n->parent && k == n->parent->node)
				continue;

			LssNode *kid = lssNodes.find(k->state);
			if (!kid) {
				kid = lssPool.construct();
				kid->node = k;
				kid->g = geom2d::Infinity;
				kid->openind = -1;
				lssNodes.add(kid);
				nnodes++;
			}

			if (kid->g <= n->g + e.outcost)
				continue;

			if (kid->parent)	// !NULL if a dup
				this->res.dups++;

			kid->parent = n;
			kid->g = n->g + e.outcost;
			kid->f = kid->g + kid->node->h;
			kid->op = e.op;

			if (n->frontier >= 0) {	// remove from frontier
				auto back = lssFrontier.back();
				lssFrontier[n->frontier] = back;
				lssFrontier.pop_back();
				back->frontier = n->frontier;
				n->frontier = -1;
			}

			dfs(d, depth+1, kid);
		}
	}

	void hCostLearning(D &d) {
		BinHeap<typename LssNode::HSort, LssNode*> open;

		open.append(lssFrontier);
		unsigned long nclosed = nnodes - lssFrontier.size();

		while (nclosed > 0 && !open.empty()) {
			LssNode *s = *open.pop();

			if (s->frontier < 0)
				nclosed--;

			for (auto e : s->node->preds) {
				Node *sprime = e.node;

				LssNode *sp = lssNodes.find(sprime->state);
				if (!sp || sp->frontier >= 0)
					continue;

				if (!sp->updated || sprime->h > e.incost + s->node->h) {
					sprime->h = e.incost + s->node->h;
					sp->updated = true;
					open.pushupdate(sp, sp->openind);
				}
			}
		}
	}

	bool better(const LssNode *a, const LssNode *b) const {
		if (geom2d::doubleeq(a->f, b->f))
			return a->g > b->g;
		return a->f < b->f;
	}

	Node *move(Node *cur, LssNode *goal) {
		LssNode *best = goal;
		if (!best) {
			for (auto n : lssFrontier) {
				if (n->node == cur)
					continue;
				if (best == NULL || better(n, best))
					best = n;
			}
		}
		assert (best);
		assert (best->node != cur);
		std::vector<Oper> ops;
		for (LssNode *p = best; p->node != cur; p = p->parent) {
			assert (p->parent != best);	// no cycles
			ops.push_back(p->op);
		}
		this->res.ops.insert(this->res.ops.end(), ops.rbegin(), ops.rend());
		lengths.push_back(ops.size());
		return best->node;
	}

	// Expand returns the successor nodes of a state.
	std::vector<outedge> expand(D &d, Node *n) {
		assert(!n->dead);

		if (n->expd)
			return n->succs;

		State buf, &s = d.unpack(buf, n->state);

		this->res.expd++;

		Operators ops(d, s);
		for (unsigned int i = 0; i < ops.size(); i++) {
			this->res.gend++;

			Edge e(d, s, ops[i]);
			Node *k = nodes.get(d, e.state);
			k->preds.emplace_back(n, e.cost);
			n->succs.emplace_back(k, ops[i], e.revcost, e.cost);
		}
		n->expd = true;

		return n->succs;
	}

	std::vector<LssNode*> lssFrontier;
	ClosedList<typename LssNode::Nodes, LssNode, D> lssNodes;
	Pool<LssNode> lssPool;
	unsigned int nclosed;
	double alpha;
	unsigned long nexpd, nnodes;

	double avgexp;

	Nodes nodes;
	unsigned int depthlimit;

	std::vector<double> times;
	std::vector<unsigned int> lengths;
};