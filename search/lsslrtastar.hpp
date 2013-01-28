#pragma once
#include "../search/search.hpp"
#include "../utils/pool.hpp"

template <class D> struct Lsslrtastar : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;
	typedef typename D::Operators Operators;
	typedef typename D::Edge Edge;

	struct Node;

	struct predecessor {
		predecessor(Node* p, double c) : pred(p), cost(c) {}
		Node* pred;
		double cost;
	};

	struct Node {
		ClosedEntry<Node, D> closedent;
		int openind;
		Node *parent;
		PackedState state;
		Oper op, pop;
		double f, g, h;
		unsigned int iterationCount;
		std::vector<predecessor> predecessors;

		Node() : openind(-1), iterationCount(0) {
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
				return a->g > b->g;
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

	Lsslrtastar(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), seen(30000001), lssclosed(1), iterationCount(0), oneStep(false) {
		lookahead = 0;

		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-lookahead") == 0)
				lookahead = strtod(argv[++i], NULL);
			else if(i < argc - 1 && strcmp(argv[i], "-onestep") == 0)
				oneStep = true;
		}

		if (lookahead < 1)
			fatal("Must specify a lookahead ≥1 using -lookahead");


		lssclosed.resize(lookahead * 3);

		nodes = new Pool<Node>();
	}

	~Lsslrtastar() {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {

		this->start();

		lssclosed.init(d);

		Node* start = init(d, s0);
		seen.add(start);
		foreverStart = start;

		State buf, &startState = d.unpack(buf, start->state);

		while(!d.isgoal(startState) && !this->limit()) {

			Node* s_goal = astar(d, start);

			if(s_goal == NULL) break;

			if (!this->limit())
				dijkstra(d);

			std::vector<Oper> partial;
			Node* p = s_goal;

			emitTimes.push_back(cputime() - this->res.cpustart);

			Node* oneStepState = NULL;
			while(start != p) {
				//or until the edge costs change?
				assert(p->parent);
				partial.push_back(p->op);
				oneStepState = p;
				p = p->parent;
			}

			if(oneStep) {
				lengths.push_back(1);
				this->res.ops.push_back(partial.back());
				start = oneStepState;
			}
			else {
				lengths.push_back(partial.size());
				this->res.ops.insert(this->res.ops.end(), partial.rbegin(), partial.rend());
				start = s_goal;
			}

			//update action costs?

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

	Node* astar(D& d, Node* s_start) {
		iterationCount++; //handle setting g's to inf by checking iteration number
		lssclosed.clear();
		lssopen.clear();

		Node* goal = NULL;

		s_start->parent = NULL;
		s_start->iterationCount = iterationCount;
		s_start->g = 0;
		s_start->pop = D::Nop;

		lssopen.push(s_start);

		for(unsigned int exp = 0; exp < lookahead && !lssopen.empty() && !this->limit(); exp++) {
			Node* s = *lssopen.pop();

			if(goal != NULL && goal->f <= s->f) {
				lssopen.push(s);
				return goal;
			}

			State buf, &state = d.unpack(buf, s->state);

			unsigned long hash = s->state.hash(&d);

			if(!lssclosed.find(s->state, hash)) {
				lssclosed.add(s, hash);
			}

			Operators ops(d, state);
			this->res.expd++;
			for(unsigned int i = 0; i < ops.size(); i++) {

				Edge edge(d, state, ops[i]);
				Node *kid = nodes->construct();

				this->res.gend++;
				d.pack(kid->state, edge.state);

				unsigned long hash = kid->state.hash(&d);
				Node *dup = seen.find(kid->state, hash);
				double newG = s->g + edge.cost;

				if(dup) {
					nodes->destruct(kid);
					kid = dup;

					if(kid->iterationCount < iterationCount) {
						kid->g = std::numeric_limits<double>::infinity();
						kid->predecessors.clear();
						kid->openind = -1;
					}

					if(kid->g > newG) {
						if (dup)
							this->res.dups++;
						kid->g = newG;
						kid->parent = s;
						kid->f = kid->g + kid->h;
						kid->op = ops[i];
						kid->pop = edge.revop;

						lssopen.pushupdate(kid, kid->openind);
					}
				}
				else {
					kid->g = newG;
					kid->h = d.h(edge.state);
					kid->parent = s;
					kid->f = kid->g + kid->h;
					kid->op = ops[i];
					kid->pop = edge.revop;

					seen.add(kid, hash);
					lssopen.push(kid);

					if(d.isgoal(edge.state) && (goal == NULL || kid->g < goal->g)) {
						goal = kid;
					}
				}

				kid->iterationCount = iterationCount;
				kid->predecessors.emplace_back(s, edge.cost);
			}
		}

		if(lssopen.empty())
			return NULL;

		return *lssopen.front();
	}

	void dijkstra(D& d) {
		iterationCount++;

		BinHeap<HSort, Node*> open;

		open.append(lssopen.data());

		while(lssclosed.getFill() > 0) {

			Node* s = *open.pop();

			unsigned long hash = s->state.hash(&d);
			if(lssclosed.find(s->state, hash)) {
				lssclosed.remove(s->state, hash);
			}

			std::vector<predecessor>& pred = s->predecessors;

			for(unsigned int i = 0; i < pred.size(); i++) {
				Node* p = pred[i].pred;

				bool inClosed = lssclosed.find(p->state) != NULL;

				if(inClosed && p->iterationCount < iterationCount) {
					p->iterationCount = iterationCount;
					p->h = std::numeric_limits<double>::infinity();
				}

				double newH = s->h + pred[i].cost;
				if(inClosed && p->h > newH) {
					p->h = newH;
					open.pushupdate(p, p->openind);
				}
			}
		}
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
	unsigned int iterationCount;
	Node* foreverStart;
	std::vector<double> emitTimes;
	std::vector<unsigned int> lengths;
	bool oneStep;
};
