// Copyright © 2020 the Search Authors under the MIT license. See AUTHORS for the list of authors.                                                             
#pragma once                                                                    
#include "../search/search.hpp"                                                 
#include "../utils/pool.hpp"
                                                                                
template <class D> struct BeamSearch : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;                                
	typedef typename D::Cost Cost;                                              
	typedef typename D::Oper Oper;

	struct Node {
		int openind;
		Node *parent;
		PackedState state;
		Oper op, pop;
		Cost f, g;

		Node() : openind(-1) {
		}

		static ClosedEntry<Node, D> &closedentry(Node *n) {
			return n->closedent;
		}

		static PackedState &key(Node *n) {
			return n->state;
		}

		/* Set index of node on open list. */
		static void setind(Node *n, int i) {
			n->openind = i;
		}

		/* Get index of node on open list. */
		static int getind(const Node *n) {
			return n->openind;
		}

		/* Indicates whether Node a has better value than Node b. */
		static bool pred(Node *a, Node *b) {
			if (a->f == b->f)
				return a->g > b->g;
			return a->f < b->f;
		}

		/* Priority of node. */
		static Cost prio(Node *n) {
			return n->f;
		}

		/* Priority for tie breaking. */
		static Cost tieprio(Node *n) {
			return n->g;
		}

    private:
		ClosedEntry<Node, D> closedent;
    
	};

	BeamSearch(int argc, const char *argv[]) :
		SearchAlgorithm<D>(argc, argv), closed(30000001) {
		dropdups = false;
		for (int i = 0; i < argc; i++) {
			if (i < argc - 1 && strcmp(argv[i], "-width") == 0)
				width = atoi(argv[++i]);
			if (strcmp(argv[i], "-dropdups") == 0)
				dropdups = true;
		}

		if (width < 1)
			fatal("Must specify a >0 beam width using -width");
    
		nodes = new Pool<Node>();
	}

	~BeamSearch() {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {
		this->start();
		closed.init(d);

		Node *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		int depth = 0;


		bool solved = false;
    
		while (!open.empty() && !solved && !SearchAlgorithm<D>::limit()) {
			depth++;
      
			Node **beam = new Node*[width];
			int c;
			for(c = 0; c < width && !open.empty(); c++) {
				Node *n = open.pop();

				beam[c] = n;
			}

			open.clear();
      
			for(int i = 0; i < c && !solved && !SearchAlgorithm<D>::limit(); i++) {
				Node *n = beam[i];
				State buf, &state = d.unpack(buf, n->state);

				if (d.isgoal(state)) {
					solpath<D, Node>(d, n, this->res);
					solved = true;
					break;
				}

				expand(d, n, state);
			}

			delete[] beam;
		}
		this->finish();
	}

	virtual void reset() {
		SearchAlgorithm<D>::reset();
		open.clear();
		closed.clear();
		delete nodes;
		nodes = new Pool<Node>();
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", open.kind());
		dfpair(stdout, "node size", "%u", sizeof(Node));
	}


private:

	void expand(D &d, Node *n, State &state) {
		SearchAlgorithm<D>::res.expd++;

		typename D::Operators ops(d, state);
		for (unsigned int i = 0; i < ops.size(); i++) {
			if (ops[i] == n->pop)
				continue;
			SearchAlgorithm<D>::res.gend++;
			considerkid(d, n, state, ops[i]);
		}
	}

	void considerkid(D &d, Node *parent, State &state, Oper op) {
		Node *kid = nodes->construct();
		assert (kid);
		typename D::Edge e(d, state, op);
		kid->g = parent->g + e.cost;
		d.pack(kid->state, e.state);

		unsigned long hash = kid->state.hash(&d);
		Node *dup = closed.find(kid->state, hash);
		if (dup) {
			this->res.dups++;
			if (dropdups || kid->g >= dup->g) {
				nodes->destruct(kid);
				return;
			}
			bool isopen = open.mem(dup);
			if (isopen)
				open.pre_update(dup);
			dup->f = dup->f - dup->g + kid->g;
			dup->g = kid->g;
			dup->parent = parent;
			dup->op = op;
			dup->pop = e.revop;
			if (isopen) {
				open.post_update(dup);
			} else {
				this->res.reopnd++;
				open.push(dup);
			}
			nodes->destruct(kid);
		} else {
			kid->f = kid->g + d.h(e.state);
			kid->parent = parent;
			kid->op = op;
			kid->pop = e.revop;
			closed.add(kid, hash);
			open.push(kid);
		}
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->state, s0);
		n0->g = Cost(0);
		n0->f = d.h(s0);
		n0->pop = n0->op = D::Nop;
		n0->parent = NULL;
		return n0;
	}

    int width;
    bool dropdups;
	OpenList<Node, Node, Cost> open;
 	ClosedList<Node, Node, D> closed;
	Pool<Node> *nodes;
  
};
