#pragma once
#include "../search/search.hpp"
#include "../utils/pool.hpp"
#include "../utils/geom2d.hpp"


template <class D> struct Flrtastar : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef typename D::Operators Operators;
	typedef typename D::Edge Edge;

	struct Node;

	struct prededge {
		prededge(Node* n, double c) : node(n), costFrom(c) {}
		Node* node;
		double costFrom;
	};

	struct succedge {
		succedge(Node* n, Oper o, double c) : node(n), op(o), costTo(c) {}
		Node* node;
		Oper op;
		double costTo;
	};

	struct Node {
		ClosedEntry<Node, D> closedent;
		int openind;
		Node *parent;
		PackedState state;
		Oper op, pop;
		double f, g, h, g_local;
		unsigned int iterationCount;
		std::vector<prededge> preds;
		std::vector<succedge> succs;
		bool dead;

		Node() : openind(-1), iterationCount(0), dead(false) {
		}

		static ClosedEntry<Node, D> &closedentry(Node *n) {
			return n->closedent;
		}

		static PackedState &key(Node *n) {
			return n->state;
		}

		static void setind(Node *n, int i) {
			n->openind = i;
		}

		static bool pred(const Node *a, const Node *b) {
			if (a->f == b->f)
				return a->g_local > b->g_local;
			return a->f < b->f;
		}
	};

	struct HSort {
		static void setind(Node *n, int i) {
			n->openind = i;
		}

		static bool pred(const Node *a, const Node *b) {
			return a->h < b->h;
		}
	};

	struct GSort {
		static void setind(Node *n, int i) {
			n->openind = i;
		}

		static bool pred(const Node *a, const Node *b) {
			return a->g < b->g;
		}
	};

	Flrtastar(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), seen(30000001), lssclosed(1), iterationCount(0) {

		lookahead = 0;
		weight = 0;

		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-lookahead") == 0)
				lookahead = strtod(argv[++i], NULL);
			else if (i < argc - 1 && strcmp(argv[i], "-wt") == 0)
				weight = strtod(argv[++i], NULL);
		}

		if (lookahead < 1)
			fatal("Must specify a lookahead ≥1 using -lookahead");

		if (weight < 1)
			fatal("Must specify a weight ≥1 using -wt");

		lssclosed.resize(lookahead * 3);

		nodes = new Pool<Node>();
	}

	~Flrtastar() {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {
		this->start();

		lssclosed.init(d);

		Node* start = init(d, s0);
		seen.add(start);

		State buf, &startState = d.unpack(buf, start->state);

		while(!d.isgoal(startState) && !this->limit()) {
			Node* dest = expandLSS(d, start);

			if(!dest && !this->limit()) { //we didn't find a goal
				gCostLearning(d);

				lsslrtastarHCostLearning(d);

				dest = markDead(d); // returns NULL if all of open is dead, or best node otherwise
			}
//			else fprintf(stderr, "expandLSS\n");

			if(!dest) { //all of open was dead
				dest = getNeighborWithLowestG(d, start);
//				fprintf(stderr, "neighbor\n");
			}
//			else fprintf(stderr, "markDead\n");

			std::vector<Oper> partial;
			Node* p = dest;

			while(start != p) {
				//or until the edge costs change?
				assert(p->parent);
				partial.push_back(p->op);
				p = p->parent;
			}
			lengths.push_back(partial.size());

//			testMoves(d, start, dest, partial);

			this->res.ops.insert(	this->res.ops.end(), partial.rbegin(), partial.rend());

			start = dest;

			emitTimes.push_back(cputime() - this->res.cpustart);

			startState = d.unpack(buf, start->state);
		}

		this->finish();

		if (!d.isgoal(startState)) {
			this->res.ops.clear();
			return;
		}

		// Rebuild the path from the operators to avoid storing very long
		// paths as we go.
		seen.clear();
		this->res.path.push_back(s0);
		for (auto it = this->res.ops.begin(); it != this->res.ops.end(); it++) {
			State copy = this->res.path.back();
			typename D::Edge e(d, copy, *it);
			this->res.path.push_back(e.state);
		}

		std::reverse(this->res.ops.begin(), this->res.ops.end());
		std::reverse(this->res.path.begin(), this->res.path.end());
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		dfpair(out, "num steps", "%lu", emitTimes.size());
		if (emitTimes.size() > 0) {
			dfpair(out, "first emit cpu time", "%f", emitTimes[0]);
			double minEmit =  emitTimes.front();
			double maxEmit = emitTimes.front();
			for(unsigned int i = 1; i < emitTimes.size(); i++) {
				double stepTime = emitTimes[i] - emitTimes[i-1];
				if(stepTime < minEmit) minEmit = stepTime;
				if(stepTime > maxEmit) maxEmit = stepTime;
			}
			dfpair(out, "min step cpu time", "%f", minEmit);
			dfpair(out, "max step cpu time", "%f", maxEmit);
			dfpair(out, "mean step cpu time", "%f", (emitTimes.back() - emitTimes.front()) / (double) emitTimes.size());
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

	virtual void reset() {
		fatal("reset not implemented");
	}

private:

	void testMoves(D& d, Node* start, Node* end, std::vector<Oper> &path) {
		Node* s = start;
		Node n;
		for(auto iter = path.rbegin();iter != path.rend(); iter++) {
			Oper op = *iter;
			State buf, &state = d.unpack(buf, s->state);
			Edge edge(d, state, op);

			d.pack(n.state, edge.state);

			s = seen.find(n.state);
			assert(s);
		}
		if(s != end) fprintf(stderr, "path length: %d\n", (int)path.size());
		assert(s == end);
	}

	//if a goal is found, it is be returned, otherwise NULL
	Node* expandLSS(D& d, Node* s_cur) {
		iterationCount++;

		lssopen.clear();
		lssclosed.clear();

		Node* goal = NULL;

		s_cur->g_local = 0;
		s_cur->parent = NULL;
		s_cur->iterationCount = iterationCount;
		s_cur->pop = D::Nop;

		lssopen.push(s_cur);

		for(unsigned int exp = 0; exp < lookahead && !lssopen.empty() && !this->limit(); exp++) {

			Node* s  = *lssopen.pop();

			if(goal != NULL && goal->f <= s->f) {
				lssopen.push(s);
				return goal;
			}

			unsigned long hash = s->state.hash(&d);
			if(!lssclosed.find(s->state, hash))
				lssclosed.add(s, hash);

			if(s->dead) continue;

			expandAndPropagate(d, s, &goal, true);
		}

		return NULL;
	}

	void expandAndPropagate(D& d, Node* s, Node** goal, bool expand, bool debug=false) {

		if (this->limit())
			return;

		State buf, &state = d.unpack(buf, s->state);
		Operators ops(d, state);
		bool populateSuccessors = s->succs.size() == 0;

		this->res.expd++;
		for(unsigned int i = 0; i < ops.size(); i++) {

			Edge e(d, state, ops[i]);
			Node *kid = nodes->construct();

			this->res.gend++;
			d.pack(kid->state, e.state);

			unsigned long hash = kid->state.hash(&d);
			Node *dup = seen.find(kid->state, hash);

			double newLocalG = s->g_local + e.cost;

			if(expand) {
				if(dup) {
					nodes->destruct(kid);
					kid = dup;

					if(kid->iterationCount < iterationCount) {
						kid->g_local = std::numeric_limits<double>::infinity();
//						kid->preds.clear();
						kid->openind = -1;
					}

					if(kid->g_local > newLocalG) {
						if (dup)
							this->res.dups++;
						kid->g_local = newLocalG;
						kid->parent = s;
						kid->f = kid->g_local + kid->h;
						kid->op = ops[i];
						kid->pop = e.revop;

						lssopen.pushupdate(kid, kid->openind);
					}
				}
				else {
					kid->g_local = newLocalG;
					kid->g = s->g + e.cost;
					kid->h = d.h(e.state);
					kid->parent = s;
					kid->f = kid->g_local + kid->h;
					kid->op = ops[i];
					kid->pop = e.revop;

					seen.add(kid, hash);
					lssopen.push(kid);


					if(d.isgoal(e.state) && (*goal == NULL || kid->g_local < (*goal)->g_local)) {
						*goal = kid;
					}
				}

				kid->iterationCount = iterationCount;
				bool found = false;
				for(unsigned int j = 0; j < kid->preds.size(); j++) {
					if(kid->preds[j].node == s) { found = true; break; }
				}
				if(!found)
					kid->preds.emplace_back(s, e.cost);
				if(populateSuccessors)
					s->succs.emplace_back(kid, ops[i], e.cost);
			}
			else {
				nodes->destruct(kid);
				if(!dup) continue;
				kid = dup;
			}

			double newGCost = s->g + e.cost;

			if(kid->g > newGCost) {
				bool wasDead = kid->dead;
				kid->dead = false;
				kid->g = newGCost;
//				kid->h = d.h(e.state); //only for optimality over trials
				//pushupdate if on open? Nathan doesn't

				bool onClosed = lssclosed.find(kid->state) != NULL;
				if(onClosed && wasDead) {
					expandAndPropagate(d, kid, goal, true);
				}
				else if(onClosed || kid->openind >= 0) {
					expandAndPropagate(d, kid, goal, false);
				}
			}

			double revEdgeCost = e.revcost == Cost(-1) ? std::numeric_limits<double>::infinity() : e.revcost;

			if(!kid->dead && s->g > kid->g + revEdgeCost) {
				s->g = kid->g + revEdgeCost;
//				s->h = d.h(state);
				if(i != 0) i = -1;
			}
		}
	}

	void gCostLearning(D& d) {
		Node* dummy = NULL; // this is ignored
		std::vector<Node*> &openData = lssopen.data();
		for(unsigned int i = 0; i < openData.size(); i++) {
			expandAndPropagate(d, openData[i], &dummy, false);
		}
	}

	void lsslrtastarHCostLearning(D& d) {
		iterationCount++;

		BinHeap<HSort, Node*> open;

		open.append(lssopen.data());

		std::vector<Node*> oldClosed;

		while(lssclosed.getFill() > 0) {

//TODO: Nathan does this and we needed it to but I'm not convinced it is correct
//with respect to the original LSSLRTA* Algorithm
			if(open.size() == 0) break;

			Node* s = *open.pop();

			unsigned long hash = s->state.hash(&d);
			if(lssclosed.find(s->state, hash)) {
				oldClosed.push_back(lssclosed.remove(s->state, hash));
			}

			for(unsigned int i = 0; i < s->preds.size(); i++) {
				Node* p = s->preds[i].node;

				bool inClosed = lssclosed.find(p->state) != NULL;

				if(inClosed && p->iterationCount < iterationCount) {
					p->iterationCount = iterationCount;
					p->h = std::numeric_limits<double>::infinity();
				}

				double newH = s->h + s->preds[i].costFrom;
				if(inClosed && p->h > newH) {
					p->h = newH;
					open.pushupdate(p, p->openind);
				}
			}
		}

		for(unsigned int i = 0; i < oldClosed.size(); i++) {
			lssclosed.add(oldClosed[i]);
		}
	}

	Node* markDead(D& d) {
int count = 0;
		for(auto s : lssclosed) {
			if(s->dead) continue;
			s->dead = isDeadEnd(d, s) || isRedundant(d, s);
			if(s->dead) count++;
		}

		Node* minNode = NULL;
//		double minCost = std::numeric_limits<double>::infinity();
		bool openIsDead = true;
		auto openData = lssopen.data();
		for(auto s : openData) {
			if(s->dead) continue;

			s->dead =  isDeadEnd(d, s) || isRedundant(d, s);
			if(s->dead) count++;

			if(s->dead) continue;

			openIsDead = false;

//			double cost = s->g + weight * s->h;
//			double cost = s->g_local + weight * s->h;
			if(minNode == NULL || Node::pred(s, minNode)) { //cost < minCost) {
				minNode = s;
//				minCost = cost;
			}
		}

		if(openIsDead)
			return NULL;

		return minNode;
	}

	bool isDeadEnd(D& d, Node* s) {
		State buf, &state = d.unpack(buf, s->state);
		Operators ops(d, state);

		if(ops.size() == 0) return true;

		this->res.expd++;
		for(unsigned int i = 0; i < ops.size(); i++) {

			Edge edge(d, state, ops[i]);
			PackedState key;

			this->res.gend++;
			d.pack(key, edge.state);

			Node *kid = seen.find(key);

			double value = s->g + edge.cost;
			if(!kid || kid->g >= value)
				return false;
		}

		return true;
	}

	bool isRedundant(D& d, Node* s) {
		State buf, &state = d.unpack(buf, s->state);
		Operators ops(d, state);

		if(ops.size() == 0) return true;

		this->res.expd++;
		for(unsigned int i = 0; i < ops.size(); i++) {

			Edge edge(d, state, ops[i]);
			PackedState key;

			this->res.gend++;
			d.pack(key, edge.state);

			Node *kid = seen.find(key);

			if(!kid)
				return false;
			else if(kid->dead)
				continue;


			Node* minNode = NULL;
			double minCost = std::numeric_limits<double>::infinity();

			for(unsigned int j = 0; j < kid->preds.size(); j++) {

			 	const prededge &p = kid->preds[j];

				if(p.node->dead)
					continue;

				double cost = p.node->g + p.costFrom;

				if(minNode == NULL || minCost > cost) {
					minCost = cost;
					minNode = p.node;
				}
			}

			if(s == minNode)
				return false;
		}

		return true;
	}

	Node* getNeighborWithLowestG(D& d, Node* s) {
		if(s->succs.size() > 0) {
//			fprintf(stderr, "cached\n");
			Node* minGKid = s->succs[0].node;
			minGKid->op = s->succs[0].op;

			for(unsigned int i = 1; i < s->succs.size(); i++) {
				Node *kid = s->succs[i].node;

				if(kid->g < minGKid->g) {
					minGKid = kid;
					minGKid->op = s->succs[i].op;
				}
			}

			minGKid->parent = s;

			return minGKid;
		}
//fprintf(stderr, "generated\n");
		Node* minGKid = NULL;
		double minGCost = std::numeric_limits<double>::infinity();

		State buf, &state = d.unpack(buf, s->state);
		Operators ops(d, state);
		this->res.expd++;
		for(unsigned int i = 0; i < ops.size(); i++) {

			Edge edge(d, state, ops[i]);
			Node *kid = nodes->construct();

			this->res.gend++;
			d.pack(kid->state, edge.state);

			unsigned long hash = kid->state.hash(&d);
			Node *dup = seen.find(kid->state, hash);

			if(dup) {
				nodes->destruct(kid);
				kid = dup;
				kid->op = ops[i];
				kid->pop = edge.revop;
				kid->parent = s;
			}
			else {
				kid->g = s->g + edge.cost;
				kid->h = d.h(edge.state);
				kid->f = kid->g + kid->h;
				kid->op = ops[i];
				kid->pop = edge.revop;
				kid->parent = s;

				seen.add(kid, hash);
			}

			if(kid->g < minGCost) {
				minGKid = kid;
				minGCost = kid->g;
			}
			s->succs.emplace_back(kid, ops[i], edge.cost);
		}

		assert(minGKid != NULL);
		return minGKid;
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->state, s0);
		n0->g = 0;
		n0->f = d.h(s0);
		n0->pop = n0->op = D::Nop;
		n0->parent = NULL;
		return n0;
	}

 	ClosedList<Node, Node, D> seen;
	BinHeap<Node, Node*> lssopen;
 	ClosedList<Node, Node, D> lssclosed;
	Pool<Node> *nodes;
	unsigned int lookahead;
	double weight;
	unsigned int iterationCount;
	std::vector<double> emitTimes;
	std::vector<unsigned int> lengths;
};
