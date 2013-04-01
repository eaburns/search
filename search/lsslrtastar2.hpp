#pragma once
#include "../search/search.hpp"
#include "../utils/geom2d.hpp"
#include "../utils/pool.hpp"
#include "../utils/utils.hpp"
#include "../rdb/rdb.hpp"
#include <vector>

class LookaheadLimit {
public:
	virtual void start(double g) = 0;
	virtual bool stop() = 0;
	virtual void output(FILE *out) const = 0;

	static LookaheadLimit *fromArgs(int argc, const char *argv[]);
};

template <class D>
class Lsslrtastar2 : public SearchAlgorithm<D> {
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

	Lsslrtastar2(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv),
		lssNodes(4051),
		nodes(30000001),
		complete(false),
		onestep(false) {

		lsslim = LookaheadLimit::fromArgs(argc, argv);

		for (int i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-onestep") == 0)
				onestep = true;
			else if (strcmp(argv[i], "-complete") == 0)
				complete = true;
		}
	}

	~Lsslrtastar2() {
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

		lsslim->start(0);

		while (!cur->goal && !this->limit()) {
			LssNode *goal = expandLss(d, cur);
			if (this->limit())
				break;
			if (!goal)
				hCostLearning(d);
			auto m = move(cur, goal);
			cur = m.first;
			lsslim->start(m.second);
			times.push_back(walltime() - this->res.wallstart);
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
			dfpair(out, "first emit wall time", "%f", times.front());
			dfpair(out, "min step wall time", "%f", min);
			dfpair(out, "max step wall time", "%f", max);
			dfpair(out, "mean step wall time", "%f", (times.back()-times.front())/times.size());
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
	
		if (!best && complete) {
			// Minimum f value still on open (unexpanded).
			// Due to consistency, all expanded nodes should have f ≤ minf.
			// We want to find the last *complete* layer of the onion, that is,
			// the last layer for which all nodes with that f value have been
			// expanded.  Clearly that layer's f value is not minf, because the
			// node at the front of open has f = minf, and it wasn't expanded.
			// All nodes with f < minf, however, must have been expanded
			// (Nilsson, 19foreverAgo).  So, we want to move to the node
			// on closed with the largest f (tie-breaking on high g) for which
			// f < minf and with the.  This will be the best node in the
			// last completed onion layer.
			double minf = (*lssOpen.front())->f;
	
			double layer = 0;
			for (auto n : lssNodes) {
				if (n->node == cur || !n->closed || n->f == minf || n->f < layer)
					continue;
	
				if (n->f > layer) {
					layer = n->f;
					best = n;
				}
	
				if (n->f == layer && (!best || LssNode::FSort::pred(n, best)))
					best = n;
			}
		}

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

		if (onestep) {
			this->res.ops.push_back(ops.back());
			lengths.push_back(1);
			assert (q);
			assert (q->parent);
			assert (q->parent->node == cur);
			return std::make_pair(q->node, q->g);
		}			

		this->res.ops.insert(this->res.ops.end(), ops.rbegin(), ops.rend());
		lengths.push_back(ops.size());
		return std::make_pair(best->node, best->g);
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

	BinHeap<typename LssNode::FSort, LssNode*> lssOpen;
	ClosedList<typename LssNode::Nodes, LssNode, D> lssNodes;
	Pool<LssNode> lssPool;
	unsigned int nclosed;

	LookaheadLimit *lsslim;
	Nodes nodes;

	bool complete;	// move only to best node in completed f layer
	bool onestep;

	std::vector<double> times;
	std::vector<unsigned int> lengths;
};

class ExpansionLimit : public LookaheadLimit {
public:

	ExpansionLimit(unsigned int n) : lim(n) {
	}

	virtual void start(double) {
		n = 0;
	}

	virtual bool stop() {
		return n++ >= lim;
	}

	virtual void output(FILE *out) const {
		dfpair(out, "limit type", "%s", "expansion limit");
		dfpair(out, "lookahead", "%u", lim);
	}

private:
	unsigned int n, lim;
};

static void dfMaxStepTime(std::vector<std::string>& toks, void *data) {
	if(toks[0] != "#pair" || toks[1] != "max step wall time")
		return;

	char *end = NULL;
	double t = strtod(toks[2].c_str(), &end);
	if (end == NULL)
		fatal("Unable to convert %s to a double", toks[2].c_str());

	double *max = (double*)data;
	*max = std::max(*max, t);
}

class MaxTimeLimit : public LookaheadLimit {

	struct lsslookup {
		lsslookup(unsigned int size, double t) : size(size), maxTime(t) {
		}

		bool operator<(const lsslookup &l) const {
			return size < l.size;
		}

		unsigned int size;
		double maxTime;
	};

public:

	MaxTimeLimit(const std::string &alg, const std::string &dataRoot, const std::string &levelFile, unsigned int l0, double gs) : gscale(gs), deadline(0), lim0(l0), missed(0) {
		RdbAttrs lvlAttrs = pathattrs(levelFile);
		auto dom = lvlAttrs.lookup("domain");

		RdbAttrs attrs;

		if(dom == "plat2d") {	
			attrs.push_back("alg", alg);
			attrs.push_back("onestep", "no");
			attrs.push_back("type", "training");
			attrs.push_back("width", lvlAttrs.lookup("width"));
			attrs.push_back("height", lvlAttrs.lookup("height"));

		} else if (dom == "grid_instances") {
			dom = "gridnav";
			attrs.push_back("alg", alg);
			attrs.push_back("onestep", "no");
			attrs.push_back("type", "cpp-seedinst");
			attrs.push_back("costs", lvlAttrs.lookup("costs"));
			attrs.push_back("moves", lvlAttrs.lookup("moves"));
			attrs.push_back("prob", lvlAttrs.lookup("prob"));
			attrs.push_back("width", lvlAttrs.lookup("width"));
			attrs.push_back("height", lvlAttrs.lookup("height"));

		} else if (dom == "tiles_instances") {
			dom = "tiles";
			attrs.push_back("alg", alg);
			attrs.push_back("onestep", "no");
			attrs.push_back("model", "random_walk");
			attrs.push_back("length", "1000000");
			attrs.push_back("rows", lvlAttrs.lookup("rows"));
			attrs.push_back("cols", lvlAttrs.lookup("cols"));

		} else {
			fatal("MaxTimeLimit not implemented for domain %s", dom.c_str());
		}

		std::string instanceRoot(pathcat(dataRoot, dom));
		std::vector<std::string> paths = withattrs(instanceRoot, attrs);
		if (paths.size() == 0)
			fatal("No data matching for %s in %s", attrs.string().c_str(), instanceRoot.c_str());
		for(unsigned int i = 0; i < paths.size(); i++) {
			FILE* in = fopen(paths[i].c_str(), "r");
			if(!in)
				fatalx(errno, "Failed to open %s while building lss lookup table", paths[i].c_str());

			double max = -1;
			dfread(in, dfMaxStepTime, &max, NULL);
			fclose(in);

			if (max < 0)
continue;
		//		fatal("No 'max step cpu time' key in %s", paths[i].c_str());

			RdbAttrs instanceAttrs = pathattrs(paths[i]);

			auto lastr = instanceAttrs.lookup("lookahead");
			char *end = NULL;
			unsigned int lookahead = strtol(lastr.c_str(), &end, 10);
			if (end == NULL)
				fatal("Unable to convert %s to an int", lastr.c_str());

			unsigned int i = 0;
			for( ; i < lsstable.size(); i++) {
				if(lsstable[i].size == lookahead)
					break;
			}

			if(i >= lsstable.size())
				lsstable.emplace_back(lookahead, max);
			else
				lsstable[i].maxTime = std::max(lsstable[i].maxTime, max);
		}

		if(lsstable.size() == 0)
			fatal("Failed to load any training data files for building the lss lookup table");

		std::sort(lsstable.begin(), lsstable.end());
	
	}

	virtual void start(double g) {
		n = 0;
		double now = walltime();
		if (g <= 0) {
			lim = lim0;
			return;
		} else if (deadline == 0) {
			deadline = now;
		}

		if (now > deadline)
			missed++;

		if (g > 1)
			g--;	// Be a bit more conservative.

		deadline += g * gscale;

		lim = maxlss(deadline - now);
	}

	// Maxlss returns the maximum LSS size that should be
	// searchable in the given amount of time.
	unsigned long maxlss(double t) const {
		for(unsigned int i = 0; i < lsstable.size(); i++) {
			if(t <= lsstable[i].maxTime)
				return lsstable[i].size;
		}
		return lsstable.back().size;
	}

	virtual bool stop() {
		return n++ >= lim;
	}

	virtual void output(FILE *out) const {
		dfpair(out, "limit type", "%s", "dynamic limit");
		dfpair(out, "initial lookahead", "%u", lim0);
		dfpair(out, "num missed deadlines", "%u", missed);
	}

private:
	double gscale;
	double deadline;
	unsigned int lim0;
	unsigned int n, lim;
	unsigned int missed;
	std::vector<lsslookup> lsstable;
};
