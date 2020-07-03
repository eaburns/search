// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#include "../search/search.hpp"
#include "../utils/pool.hpp"

template <class D> struct Hhatgreedy : public SearchAlgorithm<D> {

	typedef typename D::State State;
	typedef typename D::PackedState PackedState;
	typedef typename D::Cost Cost;
	typedef typename D::Oper Oper;

	struct Node {
    public:
      ClosedEntry<Node, D> closedent;
      int openind;
      Node *parent;
      PackedState state;
      double hhat, fhat;
      Oper op, pop;
      double h, g, d, f;

      Node() : openind(-1) {
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

      static int getind(const Node *n) {
        return n->openind;
      }
	
      static bool pred(Node *a, Node *b) {
        if (geom2d::doubleeq(a->hhat, b->hhat)) {
          if (geom2d::doubleeq(a->h, b->h))
            return a->g > b->g;
        }
        return a->hhat < b->hhat;
      }

      static double prio(Node *n) {
        return n->hhat;
      }

      static typename D::Cost tieprio(Node *n) {
        return n->g;
      }
	};

	Hhatgreedy(int argc, const char *argv[]) :
      SearchAlgorithm<D>(argc, argv),
      herror(0),
      derror(0),
      closed(30000001) {
		nodes = new Pool<Node>();
	}

	~Hhatgreedy() {
		delete nodes;
	}

	void search(D &d, typename D::State &s0) {
		this->start();
		closed.init(d);

		Node *n0 = init(d, s0);
		closed.add(n0);
		open.push(n0);

		while (!open.empty() && !SearchAlgorithm<D>::limit()) {
			Node *n = open.pop();
			State buf, &state = d.unpack(buf, n->state);

			if (d.isgoal(state)) {
				solpath<D, Node>(d, n, this->res);
				break;
			}

			expand(d, n, state);
		}
		this->finish();
	}

	virtual void reset() {
		SearchAlgorithm<D>::reset();
		open.clear();
		closed.clear();
		delete nodes;
		nodes = new Pool<Node>();
		herror = 0;
		derror = 0;
	}

	virtual void output(FILE *out) {
		SearchAlgorithm<D>::output(out);
		closed.prstats(stdout, "closed ");
		dfpair(stdout, "open list type", "%s", open.kind());
		dfpair(stdout, "node size", "%u", sizeof(Node));
		dfpair(out, "h error last", "%g", herror);
		dfpair(out, "d error last", "%g", derror);
	}

private:

	void expand(D &d, Node *n, State &state) {
		SearchAlgorithm<D>::res.expd++;

        double herrnext = 0;
		double derrnext = 0;

        Node *bestkid = NULL;
        
		typename D::Operators ops(d, state);
		for (unsigned int i = 0; i < ops.size(); i++) {
			if (ops[i] == n->pop)
				continue;
			SearchAlgorithm<D>::res.gend++;

            Node *kid = nodes->construct();

            typename D::Edge e(d, state, ops[i]);
            kid->g = n->g + e.cost;
            d.pack(kid->state, e.state);

            unsigned long hash = kid->state.hash(&d);
            Node *dup = closed.find(kid->state, hash);
            if (dup) {
              this->res.dups++;
              nodes->destruct(kid);
            } else {
              kid->h = d.h(e.state);
              kid->d = d.d(e.state);
              kid->f = kid->g + kid->h;
              double dhat = kid->d / (1 - derror);
              double hhat = kid->h + (herror * dhat);
              kid->hhat = hhat;
              kid->fhat = kid->g + kid->hhat;
              assert ((double) kid->h >= 0);
              assert ((double) kid->hhat >= 0);
              kid->parent = n;
              kid->op = ops[i];
              kid->pop = e.revop;
              closed.add(kid, hash);
              open.push(kid);

              if (!bestkid || kid->hhat < bestkid->hhat)
                bestkid = kid;
            }
		}

        

        if (bestkid) {
          double herr =  bestkid->f - n->f;
          if (herr < 0)
            herr = 0;
          double pastErrSum = herror * ((SearchAlgorithm<D>::res.expd)+imExp-1);
          herrnext = (herr + pastErrSum)/((SearchAlgorithm<D>::res.expd)+imExp);
          // imagine imExp of expansions with no error
          // regulates error change in beginning of search

          double derr = (bestkid->d+1) - n->d;
          if (derr < 0)
            derr = 0;
          if (derr >= 1)
            derr = 1 - geom2d::Threshold;
          double pastDSum = derror * ((SearchAlgorithm<D>::res.expd)+imExp-1);
          derrnext = (derr + pastDSum)/((SearchAlgorithm<D>::res.expd)+imExp);
          // imagine imExp of expansions with no error
          // regulates error change in beginning of search

          herror = herrnext;
          derror = derrnext;
        }
	}

	Node *init(D &d, State &s0) {
		Node *n0 = nodes->construct();
		d.pack(n0->state, s0);
		n0->g = Cost(0);
		n0->h = d.h(s0);
		n0->f = n0->h + n0->g;
		n0->hhat = n0->h;
		n0->fhat = n0->f;
		n0->d = d.d(s0);
		n0->op = n0->pop = D::Nop;
		n0->parent = NULL;
		return n0;
	}

	double herror;
	double derror;

	OpenList<Node, Node, double> open;
 	ClosedList<Node, Node, D> closed;
	Pool<Node> *nodes;

    int imExp = 10;
};
