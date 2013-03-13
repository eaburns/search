#pragma once
#include "../search/search.hpp"
#include "../utils/geom2d.hpp"
#include "../utils/pool.hpp"
#include "../rdb/rdb.hpp"
#include <vector>
#include <climits>

template <class D>
class Monstar_dump : public SearchAlgorithm<D> {
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
		double h, horig;
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
			n->h = n->horig = d.h(s);
			tbl.add(n, hash);
			return n;
		}

		void output(FILE *out) {
			tbl.prstats(out, "nodes ");
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

		class FSort {
		public:
			static void setind(LssNode *n, int i) {
				n->openind = i;
			}
		
			static bool pred(LssNode *a, LssNode *b) {	
				if (geom2d::doubleeq(a->f, b->f))
					return a->g > b->g;
				return a->f < b->f;
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
		bool closed;

	private:
		ClosedEntry<LssNode, D> nodesent;
	};

public:

	Monstar_dump(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv),
		nodes(30000001),
		lssNodes(4051),
		dlook(0),
		maxlook(0) {

		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-maxlook") == 0)
				maxlook = strtoul(argv[++i], NULL, 10);
			if (i < argc - 1 && strcmp(argv[i], "-dlook") == 0)
				dlook = strtoul(argv[++i], NULL, 10);
		}
		if (dlook < 1)
			fatal("Must specify a delta lookahead ≥1 using -dlook");
		if (maxlook < 1)
			fatal("Must specify a max lookahead ≥1 using -maxlook");
	}

	~Monstar_dump() {
	}

	void reset() {
		SearchAlgorithm<D>::reset();
		nodes.clear();
		lssOpen.clear();
		lssNodes.clear();
		lssPool.releaseall();
	}

	void search(D &d, State &s0) {
		this->start();

		dfrowhdr(stdout, "iteration", 5, "number", "lookahead size", "final cost", "delta h", "final time");

		double h0 = d.h(s0);
		unsigned long n = 0;
		long look = maxlook;
		double g = std::numeric_limits<double>::infinity();

		while (look > 0 && !this->limit()) {
			double c = 0;
			nodes.clear();
			Node *cur = nodes.get(d, s0);

			while (!cur->goal && c < g) {
				LssNode *goal = expandLss(d, cur, look);
				if (this->limit())
					break;
	
				hCostLearning(d);
				auto m = move(cur, goal, g - c);
				cur = m.first;
				c += m.second;

fprintf(stderr, "lookahead=%lu, cost=%g goal=%d\n", look, m.second, cur->goal);

				if (isinf(g)) {
					g = c;
					break;
				}
			}

			double t = walltime() - this->res.wallstart;
			double hcur = nodes.get(d, s0)->h;
			double dh = hcur - h0;

			if (!cur->goal) {
				auto goal = expandLss(d, cur, UINT_MAX); // A*
				c += goal ? goal->g : 0;
				assert (this->limit() || goal);
			}

			if (this->limit())
				break;


			dfrow(stdout, "iteration", "uuggg", n++, look, c, dh, t);
			fflush(stdout);

			look -= dlook;
		}

		dfpair(stdout, "lookahead cost", "%g", g);

		this->finish();
		this->res.ops.clear();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
	}

private:

	// ExpandLss returns the cheapest  goal node if one was generated
	// and NULL otherwise.
	LssNode *expandLss(D &d, Node *root, unsigned int explim) {
		lssOpen.clear();
		lssNodes.clear();
		lssPool.releaseall();
		nclosed = 0;

		LssNode *a = lssPool.construct();
		a->node = root;
		a->parent = NULL;
		a->op = D::Nop;
		a->g = 0;
		a->f = root->h;
		lssOpen.push(a);
		lssNodes.add(a);

		LssNode *goal = NULL;

		unsigned int nexp = 0;
		while (!lssOpen.empty() && nexp++ < explim && !this->limit()) {
			LssNode *s = *lssOpen.pop();

			nclosed += !s->closed;
			s->closed = true;

			for (auto e : expand(d, s->node)) {
				Node *k = e.node;
				if (s->parent && k == s->parent->node)
					continue;

				LssNode *kid = lssNodes.find(k->state);

				if (!kid) {
					kid = lssPool.construct();
					kid->node = k;
					kid->parent = NULL;
					kid->g = geom2d::Infinity;
					lssNodes.add(kid);
				}
				if (kid->g > s->g + e.outcost) {
					if (kid->parent)	// !NULL if a dup
						this->res.dups++;
					kid->parent = s;
					kid->g = s->g + e.outcost;
					kid->f = kid->g + kid->node->h;
					kid->op = e.op;
					lssOpen.pushupdate(kid, kid->openind);
				}
				if (k->goal && (!goal || kid->g < goal->g))
					goal = kid;
			}

			if (s->node->goal) {
				lssOpen.push(s);
				goal = s;
				break;
			}
		}
fprintf(stderr, "expd=%u\n", nexp-1);

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

	std::pair<Node*, double> move(Node *cur, LssNode *goal, double gleft) {
		LssNode *best = goal;
		if (!best) {
			for (auto n : lssOpen.data()) {
				if (n->node == cur)
					continue;
				if (best == NULL || LssNode::FSort::pred(n, best))
					best = n;
			}
		}

		assert (best);
		assert (best->node != cur);

		LssNode *p;
		for (p = best; p->g > gleft && p->node != cur; p = p->parent) {
			assert (p->parent != best);	// no cycles
		}
		return std::make_pair(p->node, p->g);
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
			k->h = std::max(k->h, n->h - e.cost);
			n->succs.emplace_back(k, ops[i], e.revcost, e.cost);
		}
		n->expd = true;

		return n->succs;
	}

	Nodes nodes;

	BinHeap<typename LssNode::FSort, LssNode*> lssOpen;
	ClosedList<typename LssNode::Nodes, LssNode, D> lssNodes;
	Pool<LssNode> lssPool;
	unsigned int nclosed;

	unsigned int dlook;
	unsigned int maxlook;
};
