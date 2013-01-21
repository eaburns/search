#pragma once
#include "../search/search.hpp"
#include "../utils/geom2d.hpp"
#include "../utils/pool.hpp"
#include <vector>

template <class D>
class Flrtastar2 : public SearchAlgorithm<D> {
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
		double gglobal, h, hdef;
		bool dead;
		bool expd;	// was this expanded before?
		bool goal;
		std::vector<outedge> succs;
		std::vector<inedge> preds;

	private:
		friend class Nodes;
		friend class Pool<Node>;

		Node() : gglobal(geom2d::Infinity), dead(false), expd(false) {
		}

		ClosedEntry<Node, D> closedent;
	};

	class Nodes {
	public:
		Nodes(unsigned int sz) : tbl(sz) {
			pool = new Pool<Node>();
		}

		~Nodes() {
			delete pool;
		}

		void clear() {
			tbl.clear();
			delete pool;
			pool = new Pool<Node>();
		}

		Node *get(D &d, State &s) {
			Node *n = pool->construct();
			d.pack(n->state, s);

			unsigned long hash = d.hash(n->state);
			Node *found = tbl.find(n->state, hash);
			if (found) {
				pool->destruct(n);
				return found;
			}

			n->goal = d.isgoal(s);
			n->h = n->hdef = d.h(s);
			assert (n->goal || n->h > 0);
			tbl.add(n, hash);
			return n;
		}

	private:
		ClosedList<Node, Node, D> tbl;
		Pool<Node> *pool;
	};

	class AstarNode {
	public:

		AstarNode() : openind(-1), updated(false) {
		}
	
		class Nodes {
		public:
			static ClosedEntry<AstarNode, D> &closedentry(AstarNode *n) {
				return n->nodesent;
			}
	
			static PackedState &key(AstarNode *n) {
				return n->node->state;
			}
		};
	
		class Closed {
		public:
			static ClosedEntry<AstarNode, D> &closedentry(AstarNode *n) {
				return n->closedent;
			}
	
			static PackedState &key(AstarNode *n) {
				return n->node->state;
			}
		};

		class FSort {
		public:
			static void setind(AstarNode *n, int i) {
				n->openind = i;
			}
		
			static bool pred(AstarNode *a, AstarNode *b) {	
				if (geom2d::doubleeq(a->f, b->f))
					return a->glocal > b->glocal;
				return a->f < b->f;
			}
		};

		class HSort {
		public:
			static void setind(AstarNode *n, int i) {
				n->openind = i;
			}
		
			static bool pred(AstarNode *a, AstarNode *b) {
				return a->node->h < b->node->h;
			}
		};

		Node *node;
		AstarNode *parent;
		double glocal, f;
		Oper op;
		long openind;
		bool updated;

	private:
		ClosedEntry<AstarNode, D> closedent, nodesent;
	};

public:

	Flrtastar2(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv),
		astarClosed(1),
		astarNodes(1),
		nodes(30000001),
		lookahead(0) {

		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-lookahead") == 0)
				lookahead = strtoul(argv[++i], NULL, 10);
		}
		if (lookahead < 1)
			fatal("Must specify a lookahead ≥1 using -lookahead");

		astarPool = new Pool<AstarNode>();
		astarNodes.resize(lookahead*3);
		astarClosed.resize(lookahead*3);
	}

	void reset() {
		SearchAlgorithm<D>::reset();
		nodes.clear();
		astarOpen.clear();
		astarClosed.clear();
		astarNodes.clear();
		delete astarPool;
		astarPool = new Pool<AstarNode>();
	}

	void search(D &d, State &s0) {
		this->start();

		Node *cur = nodes.get(d, s0);
		cur->gglobal = 0;

		while (!cur->goal) {
			AstarNode *goal = expandLss(d, cur);
			if (!goal) {
				gCostLearning(d);
				hCostLearning(d);
				markDeadRedundant(d);
			}
			cur = move(cur, goal);
			times.push_back(walltime() - this->res.wallstart);
		}

		this->finish();

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
			dfpair(out, "first emit time", "%g", times.front());
			dfpair(out, "min step time", "%g", min);
			dfpair(out, "max step time", "%g", max);
			dfpair(out, "mean step time", "%g", (times.back()-times.front())/times.size());
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

	// ExpandLss returns the cheapest  goal node if one was generated
	// and NULL otherwise.
	AstarNode *expandLss(D &d, Node *rootNode) {
		astarOpen.clear();
		astarNodes.clear();
		astarClosed.clear();
		delete astarPool;
		astarPool = new Pool<AstarNode>();

		AstarNode *a = astarPool->construct();
		a->node = rootNode;
		a->parent = NULL;
		a->op = D::Nop;
		a->glocal = 0;
		a->f = rootNode->h;
		astarOpen.push(a);
		astarNodes.add(a);

		AstarNode *goal = NULL;

		unsigned int exp;
		for (exp = 0; !astarOpen.empty() && exp < lookahead; exp++) {
			AstarNode *s = *astarOpen.pop();

			if (!astarClosed.find(s->node->state))
				astarClosed.add(s);

			if (s->node->dead)
				continue;

			AstarNode *g = expandPropagate(d, s, true);
			if (g && (!goal || g->glocal < goal->glocal))
				goal = g;

			if (s->node->goal) {
				astarOpen.push(s);
				goal = s;
				break;
			}
		}
		return goal;
	}

	// ExpandPropagate returns the cheapest goal node if one is generated
	// and NULL otherwise.
	AstarNode *expandPropagate(D &d, AstarNode *s, bool doExpand) {
		AstarNode *goal = NULL;

		auto kids = expand(d, s->node);
		for (unsigned int i = 0; i < kids.size(); i++) {
			auto e = kids[i];
			Node *k = e.node;
			AstarNode *kid = astarNodes.find(k->state);

			if (doExpand && astarNodes.find(s->node->state)) {
				if (!kid) {
					kid = astarPool->construct();
					kid->node = k;
					kid->glocal = geom2d::Infinity;
					kid->openind = -1;
					astarNodes.add(kid);
				}
				if (kid->glocal > s->glocal + e.outcost) {
					if (kid->parent)	// !NULL if a dup
						this->res.dups++;
					kid->parent = s;
					kid->glocal = s->glocal + e.outcost;
					kid->f = kid->glocal + kid->node->h;
					kid->op = e.op;
					astarOpen.pushupdate(kid, kid->openind);
				}
			}

			if (k->goal && kid && (!goal || kid->glocal < goal->glocal))
				goal = kid;

			if (s->node->gglobal + e.outcost < k->gglobal) {
				bool wasDead = k->dead;
				k->dead = false;
				k->gglobal = s->node->gglobal + e.outcost;
//				k->h = k->hdef;

				bool onClosed = astarClosed.find(k->state);
				if (kid && onClosed && wasDead)
					expandPropagate(d, kid, true);

				else if (kid && (kid->openind >= 0 || onClosed))
					expandPropagate(d, kid, false);
			}

			double backCost = e.revcost;
			if (k->gglobal + backCost < s->node->gglobal && !k->dead) {
				s->node->gglobal = k->gglobal + backCost;
//				s->node->h = s->node->hdef;
				if (i > 0)
					i = -1;
			}
		}

		return goal;
	}

	void gCostLearning(D &d) {
		for (auto n : astarOpen.data()) {
			if (!n->node->dead)
				expandPropagate(d, n, false);
		}
	}

	void hCostLearning(D &d) {
		BinHeap<typename AstarNode::HSort, AstarNode*> open;

		open.append(astarOpen.data());

		unsigned long left = astarClosed.getFill();

		while (left > 0 && !open.empty()) {
			AstarNode *s = *open.pop();

			if (astarClosed.find(s->node->state))
				left--;

			for (auto e : s->node->preds) {
				Node *sprime = e.node;

				AstarNode *sp = astarClosed.find(sprime->state);
				if (!sp)
					continue;

				if (!sp->updated || sprime->h > e.incost + s->node->h) {
					sprime->h = e.incost + s->node->h;
					sp->updated = true;
					open.pushupdate(sp, sp->openind);
				}
			}
		}
	}

	void markDeadRedundant(D &d) {
		for (auto n : astarClosed) {
			assert(!n->node->goal);
			if (n->node->dead) // || n->node->goal)
				continue;
			n->node->dead = isDead(d, n->node) || isRedundant(d, n->node);
		}

		for (auto n : astarOpen.data()) {
			if (n->node->dead || n->node->goal)
				continue;
			assert (!std::isinf(n->node->gglobal));
			n->node->dead = isDead(d, n->node) || isRedundant(d, n->node);
		}
	}

	bool isDead(D &d, Node *n) {
		for (auto succ : expand(d, n)) {
			if (succ.node->gglobal >= n->gglobal + succ.outcost)
				return false;
		}
		return true;
	}

	bool isRedundant(D &d, Node *n) {
		for (auto succ : expand(d, n)) {
			if (succ.node->dead)
				continue;

			// The distinct predecessor with minimum f was the earliest
			// to be expanded.
			Node *distinct = NULL;
			double bestc = geom2d::Infinity;

			for (auto pred : succ.node->preds) {
				if (pred.node->dead)
					continue;

				double c = pred.node->gglobal + pred.incost;
				if (!distinct || c < bestc) {
					distinct = pred.node;
					bestc = c;
				}
			}

			if (distinct == n)
				return false;
		}

		return true;
	}

	Node *move(Node *cur, AstarNode *goal) {
		AstarNode *best = goal;
		if (!best) {
			for (auto n : astarOpen.data()) {
				if (n->node == cur || n->node->dead)
					continue;
				if (best == NULL || AstarNode::FSort::pred(n, best))
					best = n;
			}
		}
		if (best) {
			assert (best->node != cur);
			std::vector<Oper> ops;
			for (AstarNode *p = best; p->node != cur; p = p->parent) {
				assert (p->parent != best);	// no cycles
				ops.push_back(p->op);
			}
			this->res.ops.insert(this->res.ops.end(), ops.rbegin(), ops.rend());
			lengths.push_back(ops.size());
			return best->node;
		}

		outedge next = cur->succs[0];
		for (unsigned int i = 1; i < cur->succs.size(); i++) {
			if (cur->succs[i].node->gglobal < next.node->gglobal)
				next = cur->succs[i];
		}
		this->res.ops.push_back(next.op);
		lengths.push_back(1);
		return next.node;
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

	BinHeap<typename AstarNode::FSort, AstarNode*> astarOpen;
	ClosedList<typename AstarNode::Closed, AstarNode, D> astarClosed;
	ClosedList<typename AstarNode::Nodes, AstarNode, D> astarNodes;
	Pool<AstarNode> *astarPool;

	Nodes nodes;
	unsigned int lookahead;

	std::vector<double> times;
	std::vector<unsigned int> lengths;
};