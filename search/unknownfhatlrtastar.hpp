#pragma once
#include "../search/search.hpp"
#include "../utils/geom2d.hpp"
#include "../utils/pool.hpp"
#include "lsslrtastar2.hpp"
#include <vector>

template <class D>
class UnknownFhatlrtastar : public SearchAlgorithm<D> {
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
		double h, horig, d;
		double derr;	// distance over which heuristic error applies.
		bool expd;	// was this expanded before?
		bool goal;
		std::vector<outedge> succs;
		std::vector<inedge> preds;

	private:
		friend class Nodes;
		friend class Pool<Node>;

		Node() : expd(false) {
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
			for (auto n : tbl)
				pool.destruct(n);
			tbl.clear();
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
			n->h = n->horig = d.h(s);
			n->d = n->derr = d.d(s);
			tbl.add(n, hash);
			return n;
		}

	private:
		ClosedList<Node, Node, D> tbl;
		Pool<Node> pool;
	};

	class LssNode {
	public:

		LssNode() : openind(-1), updated(false), closed(false) {
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

		class FHatSort {
		public:
			static void setind(LssNode *n, int i) {
				n->openind = i;
			}
		
			static bool pred(LssNode *a, LssNode *b) {	
				if (geom2d::doubleeq(a->fhat, b->fhat)) {
					if (geom2d::doubleeq(a->f, b->f))
						return a->g > b->g;
					return a->f < b->f;
				}
				return a->fhat < b->fhat;
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
		double g, f, fhat;
		Oper op;
		long openind;
		bool updated;
		bool closed;

	private:
		ClosedEntry<LssNode, D> nodesent;
	};

public:

	UnknownFhatlrtastar(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv),
		lssNodes(4051),
		herror(0),
		derror(0),
		nodes(30000001),
		onestep(false) {

		lsslim = LookaheadLimit::fromArgs(argc, argv);

		for (int i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-onestep") == 0)
				onestep = true;
		}
	}

	~UnknownFhatlrtastar() {
	}

	void reset() {
		SearchAlgorithm<D>::reset();
		nodes.clear();
		lssOpen.clear();
		for (auto n : lssNodes)
			lssPool.destruct(n);
		lssNodes.clear();
		herror = 0;
		derror = 0;
	}

	void search(D &d, State &s0) {
		this->start();

		Node *cur = nodes.get(d, s0);

		lsslim->start(0);

		while (!cur->goal && !this->limit()) {
			LssNode *goal = expandLss(d, cur);
			if (this->limit())
				break;
			if (!goal)
				hCostLearning(d);
			auto m = move(d, cur, goal);
			cur = m.first;
			lsslim->start(m.second);
			times.push_back(walltime() - this->res.wallstart);
			cputimes.push_back(cputime() - this->res.cpustart);
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
		dfpair(out, "num steps", "%lu", (unsigned long) times.size());
		assert (lengths.size() == times.size());
		dfpair(out, "h error last", "%g", herror);
		dfpair(out, "d error last", "%g", derror);
		if (times.size() != 0) {
			double min = times.front();
			double max = times.front();
			double cpumin = cputimes.front();
			double cpumax = cputimes.front();
			for (unsigned int i = 1; i < times.size(); i++) {
				double dt = times[i] - times[i-1];
				min = std::min(min, dt);
				max = std::max(max, dt);

				dt = cputimes[i] - cputimes[i-1];
				cpumin = std::min(cpumin, dt);
				cpumax = std::max(cpumax, dt);
			}
			dfpair(out, "first emit wall time", "%f", times.front());
			dfpair(out, "min step wall time", "%f", min);
			dfpair(out, "max step wall time", "%f", max);
			dfpair(out, "mean step wall time", "%f", (times.back()-times.front())/times.size());
			dfpair(out, "first emit cpu time", "%f", cputimes.front());
			dfpair(out, "min step cpu time", "%f", cpumin);
			dfpair(out, "max step cpu time", "%f", cpumax);
			dfpair(out, "mean step cpu time", "%f", (cputimes.back()-cputimes.front())/cputimes.size());
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
		lsslim->output(out);
	}

private:

	// ExpandLss returns the cheapest  goal node if one was generated
	// and NULL otherwise.
	LssNode *expandLss(D &d, Node *rootNode) {
		lssOpen.clear();
		for (auto n : lssNodes)
			lssPool.destruct(n);
		lssNodes.clear();
		nclosed = 0;

		LssNode *a = lssPool.construct();
		a->node = rootNode;
		a->parent = NULL;
		a->op = D::Nop;
		a->g = 0;
		a->f = rootNode->h;
		lssOpen.push(a);
		lssNodes.add(a);

		double herrnext = 0;
		double derrnext = 0;

		LssNode *goal = NULL;

		unsigned int exp = 0;
		while (!lssOpen.empty() && !lsslim->stop() && !this->limit()) {
			LssNode *s = *lssOpen.pop();

			//maybe wasn't visible before and now we should check if it's blocked
			State buf, &state = d.unpack(buf, s->node->state);
			if(d.map->blkd(state.getLoc())) {
				for(auto edge : s->node->preds) {
					for(unsigned int i = 0; i < edge.node->succs.size(); i++) {
						if(edge.node->succs[i].node == s->node) {
							edge.node->succs.erase(edge.node->succs.begin() + i);
							break;
						}
					}
				}
				continue;
			}

			nclosed += !s->closed;
			s->closed = true;
			exp++;

			LssNode *bestkid = NULL;
			for (auto e : expand(d, s->node)) {
				Node *k = e.node;
				if (s->parent && k == s->parent->node)
					continue;

				LssNode *kid = lssNodes.find(k->state);

				if (!kid) {
					kid = lssPool.construct();
					kid->node = k;
					kid->g = geom2d::Infinity;
					kid->openind = -1;
					lssNodes.add(kid);
				}
				if (kid->g > s->g + e.outcost) {
					if (kid->parent)	// !NULL if a dup
						this->res.dups++;
					kid->parent = s;
					kid->g = s->g + e.outcost;
					kid->f = kid->g + kid->node->h;

					double d = kid->node->derr / (1 - derror);
					double h = kid->node->h + herror*d;
					kid->fhat = kid->g + h;

					kid->op = e.op;
					lssOpen.pushupdate(kid, kid->openind);
				}
				if (k->goal && (!goal || kid->g < goal->g))
					goal = kid;

				if (!bestkid || kid->f < bestkid->f)
					bestkid = kid;
			}

			if (bestkid) {
				double herr =  bestkid->f - s->f;
				if (herr < 0)
					herr = 0;
				herrnext += (herr - herrnext)/(exp+1);

				// Learn derror on the original d's not the d's backed up from the fringe.
				double derr = bestkid->node->d + 1 - s->node->d;
				if (derr < 0)
					derr = 0;
				if (derr >= 1)
					derr = 1 - geom2d::Threshold;
				derrnext += (derr - derrnext)/(exp+1);
			}

			if (s->node->goal) {
				lssOpen.push(s);
				goal = s;
				break;
			}
		}

		herror = herrnext;
		derror = derrnext;

		return goal;
	}

	void hCostLearning(D &d) {
		BinHeap<typename LssNode::HSort, LssNode*> open;

		open.append(lssOpen.data());

		std::vector<Node*> updated;

		while (nclosed > 0 && !open.empty()) {
			LssNode *s = *open.pop();

			nclosed -= s->closed;

			for (auto e : s->node->preds) {
				Node *sprime = e.node;

				LssNode *sp = lssNodes.find(sprime->state);
				if (!sp || !sp->closed)
					continue;

				if (!sp->updated || sprime->h > e.incost + s->node->h) {
					sprime->h = e.incost + s->node->h;
					sprime->derr = s->node->derr;
					sprime->d = s->node->d + 1;
					if (!sp->updated)
						updated.push_back(sprime);
					sp->updated = true;
					open.pushupdate(sp, sp->openind);
				}
			}
		}

		for (auto n : updated)
			n->h = std::max(n->h, n->horig);
	}

	std::pair<Node*, double> move(D &d, Node *cur, LssNode *goal) {
		LssNode *best = goal;
		if (!best) {
			for (auto n : lssOpen.data()) {
				if (n->node == cur)
					continue;
				if (best == NULL || LssNode::FHatSort::pred(n, best))
					best = n;
			}
		}
		assert (best);
		assert (best->node != cur);
		std::vector<Oper> ops;
		std::vector<State> states;

		LssNode *q = NULL;
		for (LssNode *p = best; p->node != cur; p = p->parent) {
			assert (p->parent != best);	// no cycles
			ops.push_back(p->op);
			State buf, &s = d.unpack(buf, p->node->state);
			states.push_back(s);
			q = p;
		}

		assert (ops.size() >= 1);

		if (onestep) {
			State buf, &s = d.unpack(buf, cur->state);
			assert(!d.map->blkd(s.getLoc()));

			assert(!d.map->blkd(states.back().getLoc()));

			this->res.ops.push_back(ops.back());
			lengths.push_back(1);

			d.act(states.back(), ops.back());

			assert (q);
			assert (q->parent);
			assert (q->parent->node == cur);
			return std::make_pair(q->node, q->g);
		}

		int howMany = 0;
		for(int i = (int)ops.size() -1; i >= 0; i--) {


			if(d.map->blkd(states[i].getLoc())) {
				break;
			}

			d.act(states[i], ops[i]);

			this->res.ops.push_back(ops[i]);

			howMany++;
		}

		lengths.push_back(howMany);

		LssNode *furthest = best;
		for(int i = 0; i < (int)(ops.size() - howMany); i++)
			furthest = furthest->parent;

		return std::make_pair(furthest->node, furthest->g);
	}

	// Expand returns the successor nodes of a state.
	std::vector<outedge> expand(D &d, Node *n) {
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
			k->h = std::max(k->h, n->h - e.cost);
			n->succs.emplace_back(k, ops[i], e.revcost, e.cost);
		}
		n->expd = true;

		return n->succs;
	}

	BinHeap<typename LssNode::FHatSort, LssNode*> lssOpen;
	ClosedList<typename LssNode::Nodes, LssNode, D> lssNodes;
	Pool<LssNode> lssPool;
	unsigned int nclosed;

	double herror;
	double derror;

	LookaheadLimit *lsslim;
	Nodes nodes;

	bool onestep;

	std::vector<double> times;
	std::vector<double> cputimes;
	std::vector<unsigned int> lengths;
};