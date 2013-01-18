#pragma once
#include "../search/search.hpp"
#include "../utils/geom2d.hpp"
#include "../utils/pool.hpp"
#include <vector>

template <class D>
class Flrtastar : public SearchAlgorithm<D> {
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
		outedge(Node *n, Oper o, Oper p, double c) :
			node(n), op(o), pop(p), outcost(c) {
		}

		Node *node;
		Oper op, pop;
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

	class GSort {
	public:
		static void setind(Node *n, int i) {
		}
	
		static bool pred(Node *a, Node *b) {
			return a->gglobal < b->gglobal;
		}
	};

	class AstarNode {
	public:

		AstarNode() : openind(-1), updated(false) {
		}

		static ClosedEntry<AstarNode, D> &closedentry(AstarNode *n) {
			return n->closedent;
		}

		static PackedState &key(AstarNode *n) {
			return n->node->state;
		}

		static void setind(AstarNode *n, int i) {
			n->openind = i;
		}
	
		static bool pred(AstarNode *a, AstarNode *b) {
			if (geom2d::doubleeq(a->f, b->f))
				return a->glocal > b->glocal;
			return a->f < b->f;
		}

		Node *node;
		AstarNode *parent;
		double glocal, f;
		Oper op, pop;
		long openind;
		bool updated;

	private:
		ClosedEntry<AstarNode, D> closedent;
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

public:

	Flrtastar(int argc, const char *argv[]) :
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
			expandLss(d, cur);
			gCostLearning(d);
			hCostLearning(d);

			markDeadRedundant(d);
			cur = move(cur);
fprintf(stderr, "h=%g (Δ=%g)\n", cur->h, cur->h - cur->hdef);
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

private:

	void expandLss(D &d, Node *rootNode) {
		astarOpen.clear();
		astarNodes.clear();
		astarClosed.clear();
		delete astarPool;
		astarPool = new Pool<AstarNode>();

		AstarNode *a = astarPool->construct();
		a->node = rootNode;
		a->parent = NULL;
		a->glocal = 0;
		a->f = rootNode->h;
		a->op = a->pop = D::Nop;
		astarOpen.push(a);
		astarNodes.add(a);

		unsigned int exp;
		for (exp = 0; !astarOpen.empty() && exp < lookahead; exp++) {
			AstarNode *s = *astarOpen.pop();

			if (!astarClosed.find(s->node->state))
				astarClosed.add(s);

			if (s->node->dead)
				continue;

			expandPropagate(d, s, true);

			if (s->node->goal) {
				astarOpen.push(s);
				return;
			}
		}
	}

	void expandPropagate(D &d, AstarNode *s, bool doExpand) {
		auto kids = expand(d, s->node);
		for (unsigned int i = 0; i < kids.size(); i++) {
			auto e = kids[i];
			Node *si = e.node;

			AstarNode *kid = astarNodes.find(si->state);

			if (doExpand && astarNodes.find(s->node->state)) {
				if (!kid) {
					kid = astarPool->construct();
					kid->node = si;
					kid->glocal = geom2d::Infinity;
					astarNodes.add(kid);
				}
				if (kid->glocal > s->glocal + e.outcost) {
					kid->parent = s;
					kid->glocal = s->glocal + e.outcost;
					kid->f = kid->glocal + kid->node->h;
					kid->op = e.op;
					kid->pop = e.pop;
					astarOpen.pushupdate(kid, kid->openind);
				}
			}

			if (s->node->gglobal + e.outcost < si->gglobal) {
				bool wasDead = si->dead;
				si->dead = false;
				si->gglobal = s->node->gglobal + e.outcost;
//				si->h = si->hdef;

				bool onClosed = astarClosed.find(si->state);

				if (kid && onClosed && wasDead)
					expandPropagate(d, kid, true);

				else if (kid && doExpand && (kid->openind >= 0 || onClosed))
					expandPropagate(d, kid, false);
			}

			double backCost = geom2d::Infinity;
			if (si->gglobal + backCost < s->node->gglobal && !si->dead) {
				s->node->gglobal = si->gglobal + backCost;
//				s->node->h = s->node->hdef;
				if (i > 0)
					i = -1;
			}
		}
	}

	void gCostLearning(D &d) {
		for (auto n : astarOpen.data()) {
			if (n->node->dead)
				continue;
			expandPropagate(d, n, false);
		}
	}

	void hCostLearning(D &d) {
		BinHeap<HSort, AstarNode*> open;
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
		auto it = astarClosed.begin();
		for (AstarNode *n = it.next(); n; n = it.next()) {
			if (n->node->dead)
				continue;
			n->node->dead = isDead(d, n->node) || isRedundant(d, n->node);
		}

		for (auto n : astarOpen.data()) {
			if (n->node->dead)
				continue;
			n->node->dead = isDead(d, n->node) || isRedundant(d, n->node);
		}
	}

	bool isDead(D &d, Node *n) {
		for (auto succ : expand(d, n)) {
			if (n->gglobal + succ.outcost <= succ.node->gglobal)
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

	Node *move(Node *cur) {
		AstarNode *best = NULL;

		for (auto n : astarOpen.data()) {
			if (n->node == cur || n->node->dead)
				continue;
			if (best == NULL || AstarNode::pred(n, best))
				best = n;
		}
		if (best) {
			assert (best->node != cur);
			std::vector<Oper> ops;
			for (AstarNode *p = best; p->node != cur; p = p->parent) {
				assert (p->parent != best);	// no cycles
				ops.push_back(p->op);
			}
			this->res.ops.insert(this->res.ops.end(), ops.rbegin(), ops.rend());
fprintf(stderr, "moving %lu steps ", ops.size());
			return best->node;
		}

		outedge next = cur->succs[0];
		for (unsigned int i = 1; i < cur->succs.size(); i++) {
			if (cur->succs[i].node->gglobal < next.node->gglobal)
				next = cur->succs[i];
		}
		this->res.ops.push_back(next.op);
fprintf(stderr, "moving one step ");
		return next.node;
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
			n->succs.emplace_back(k, ops[i], e.revop, e.cost);
		}
		n->expd = true;

		return n->succs;
	}

	BinHeap<AstarNode, AstarNode*> astarOpen;
	ClosedList<AstarNode, AstarNode, D> astarClosed;
	ClosedList<AstarNode, AstarNode, D> astarNodes;
	Pool<AstarNode> *astarPool;

	Nodes nodes;
	unsigned int lookahead;
};