#pragma once
#include "../search/search.hpp"
#include "../utils/geom2d.hpp"
#include "../utils/pool.hpp"
#include "../rdb/rdb.hpp"
#include "../utils/utils.hpp"
#include <vector>

template <class D>
class Dtastar_dump : public SearchAlgorithm<D> {
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

	Dtastar_dump(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv),
		lssNodes(4051),
		nodes(30000001) {

		lsslim = LookaheadLimit::fromArgs(argc, argv);

		samplep = 0;
		horizon = 0;

		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-samplep") == 0) {
				char *end = NULL;
				samplep = strtod(argv[i+1], &end);
				if (end == argv[i+1])
					fatal("Bad -samplep value: %s", argv[i+1]);
				i++;
			} else if (i < argc - 1 && strcmp(argv[i], "-horizon") == 0) {
				horizon = strtol(argv[++i], NULL, 10);
			}
		}
		if (samplep <= 0 || samplep > 1)
			fatal("%g, Must specify sample probability in (0, 1] with -samplep");
		if (horizon == 0)
			fatal("Must specify sample lookahead horizon with -horizon");
	}

	~Dtastar_dump() {
	}

	void reset() {
		SearchAlgorithm<D>::reset();
		nodes.clear();
		lssOpen.clear();
		for (auto n : lssNodes)
			lssPool.destruct(n);
		lssNodes.clear();
	}

	void search(D &d, State &s0) {
		this->start();

		Node *cur = nodes.get(d, s0);

		dfrowhdr(stdout, "sample", 3, "h", "depth", "f");
		dfrowhdr(stdout, "branching", 1, "nkids");

		lsslim->start(0);
		while (!cur->goal && !this->limit()) {
			LssNode *goal = expandLss(d, cur);
			if (this->limit())
				break;
			if (!goal)
				hCostLearning(d);
			auto m = move(cur, goal);

			if (randgen.real() < samplep)
				evaluate(d, cur);

			cur = m.first;
			lsslim->start(m.second);
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
		nodes.output(stdout);
		lssNodes.prstats(stdout, "lss nodes ");
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

		LssNode *goal = NULL;

		while (!lssOpen.empty() && !lsslim->stop() && !this->limit()) {

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

	std::pair<Node*, double> move(Node *cur, LssNode *goal) {
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
		std::vector<Oper> ops;
		LssNode *q = NULL;
		for (LssNode *p = best; p->node != cur; p = p->parent) {
			assert (p->parent != best);	// no cycles
			ops.push_back(p->op);
			q = p;
		}

		assert (ops.size() >= 1);

		this->res.ops.push_back(ops.back());
		assert (q);
		assert (q->parent);
		assert (q->parent->node == cur);
		return std::make_pair(q->node, q->g);
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
			k->horig = std::max(k->horig, n->horig - e.cost);
			n->succs.emplace_back(k, ops[i], e.revcost, e.cost);
		}
		n->expd = true;

		return n->succs;
	}

	// Does d-ply lookahead beneath the given node and prints the minimim f value
	// backed up at each depth.
	void evaluate(D &d, Node *root) {

		for (auto n : lssNodes)
			lssPool.destruct(n);
		lssNodes.clear();
	
		LssNode *a = lssPool.construct();
		a->node = root;
		a->g = 0;
		lssNodes.add(a);

		dfrow(stdout, "branching", "u", (unsigned long) expand(d, root).size());
 
		for (unsigned long depth = 1; depth <= horizon; depth++) {

			alpha = std::numeric_limits<double>::infinity();
			dfs(d, a, root->horig, root->horig, depth);
if (alpha < a->node->horig) fprintf(stderr, "alpha=%g, h=%g\n", alpha, root->horig);
			assert (alpha >= root->horig);
			dfrow(stdout, "sample", "gug", root->horig, depth, alpha);
		}
	} 

	void dfs(D &d, LssNode *n, double f, double h, unsigned int left) {
		if (f >= alpha || left == 0) {
			alpha = std::min(alpha, f);
			return;
		}

		for (auto e : expand(d, n->node)) {
			Node *k = e.node;

			double g = n->g + e.outcost;

			LssNode *kid = lssNodes.find(k->state);
			if (!kid) {
				kid = lssPool.construct();
				kid->node = k;
				kid->g = g;
				lssNodes.add(kid);
			}

			if (kid->g == 0 || kid->g < g)
				continue;

			kid->g = g;

			double kidh = std::max(h - e.outcost, kid->node->horig);			
			double kidf = g + kidh;

			assert (kidf >= f);

			dfs(d, kid, kidf, kidh, left-1);
		}
	}

	double alpha;
	unsigned int horizon;
	double samplep;	// sample probability

	BinHeap<typename LssNode::FSort, LssNode*> lssOpen;
	ClosedList<typename LssNode::Nodes, LssNode, D> lssNodes;
	Pool<LssNode> lssPool;
	unsigned int nclosed;

	LookaheadLimit *lsslim;
	Nodes nodes;
};
