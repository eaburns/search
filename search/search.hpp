#ifndef _SEARCH_HPP_
#define _SEARCH_HPP_

#include <cstdio>
#include <vector>

#include "../structs/intpq.hpp"
#include "../structs/binheap.hpp"

#include "closedlist.hpp"

double walltime(void);
double cputime(void);
void dfpair(FILE *f, const char *key, const char *fmt, ...);

// A SearchStats structure contains statistical information
// collected during a run of a search algorithm.
struct SearchStats {
	double wallstrt, cpustrt;
	double wallend, cpuend;
	unsigned long expd, gend, reopnd, dups;

	SearchStats(void);

	// start must be called by the search in order to
	// begin collecting statistical information.
	void start(void);

	// finish must be called by the search in order to
	// stop collecting information.
	void finish(void);

	// output prints the statistical information to the given
	// file in datafile format.
	void output(FILE*);

	// add accumulated the statistical information from
	// the given search statistics in the receiver.
	void add(SearchStats &other) {
		expd += other.expd;
		gend += other.gend;
		reopnd += other.reopnd;
		dups += other.dups;

		double wt = wallend - wallstrt;
		double ct = cpuend - cpustrt;
		wallstrt = cpustrt = 0;
		wallend = wt + (other.wallend - other.wallstrt);
		cpuend = ct + (other.cpuend - other.cpustrt);
	}
};

// A Limit structure contains information about when a
// search algorithm should be stopped because it has
// hit a user specified limit.
struct Limit {
	unsigned long expd, gend;

	// reached returns true when the given statistics
	// reports that the given limit has been reached.
	bool reached(SearchStats &r) {
		return (expd > 0 && r.expd >= expd)
			|| (gend > 0 && r.gend >= gend);
	}

	Limit(void);

	Limit(int, const char*[]);

	// output prints the limit to the given file in datafile
	// format.
	void output(FILE*);
};

// SearchNode is a structure that encapsulates information that
// is shared between many search algorithms' node structures.
// A SearchNode contains parent node and operator information
// for constructing a solution path.  It also implements the operations
// for a closed list and both a bucked and priority queue open list.
template <class D> struct SearchNode {
private:
	ClosedEntry<SearchNode, D> closedent;

public:
	int openind;
	typename D::PackedState packed;
	typename D::Cost g;
	typename D::Oper op;
	typename D::Oper pop;
	SearchNode *parent;

	SearchNode(void) : openind(-1) { }

	// setind sets openind to the given value.
	static void setind(SearchNode *n, int i) { n->openind = i; }

	// entry returns a reference to the closed list entry of the
	// given node.
	static ClosedEntry<SearchNode, D> &closedentry(SearchNode *n) {
		return n->closedent;
	}

	// key returns the hash table key value for the given node.
	// The key is the packed state representation from the
	// search domain.
	static typename D::PackedState &key(SearchNode *n) {
		return n->packed;
	}

	// hash returns the hash value for a packed state representation.
	static unsigned long hash(typename D::PackedState &s) {
		return s.hash();
	}

	// eq performs an equality tests on two packed states.
	static bool eq(typename D::PackedState &a,
			typename D::PackedState &b) {
		return a.eq(b);
	}

	// update updates the g, parent, op and pop fields of
	// the receiver to match that of another node.
	void update(const SearchNode &other) {
		g = other.g;
		parent = other.parent;
		op = other.op;
		pop = other.pop;
	}
};

// An OpenList holds nodes and returns them ordered by some
// priority.  This template assumes that the nodes have a field
// 'openind'.  The Ops class has a pred method which accepts
// two Nodes and returns true if the 1st node is a predecessor
// of the second.
template <class Ops, class Node, class Cost> class OpenList {
public:
	const char *kind(void) { return "binary heap"; }

	void push(Node *n) { heap.push(n); }

	Node *pop(void) {
		boost::optional<Node*> p = heap.pop();
		if (!p)
			return NULL;
		return *p;
	}

	void pre_update(Node *n) { }

	void post_update(Node *n) {
		if (n->openind < 0)
			heap.push(n);
		else
			heap.update(n->openind);
	}

	bool empty(void) { return heap.empty(); }

	bool mem(Node *n) { return n->openind != -1; }

	void clear(void) { heap.clear(); }

private:
	struct Heapops {
 		static bool pred(Node *a, Node *b) { return Ops::pred(a, b); }

		static void setind(Node *n, int i) { n->openind = i; }
	};
	BinHeap<Heapops, Node*> heap;
};

typedef int IntOpenCost;

// This specialization of OpenList assumes that a node has an
// 'openind' field.  The Ops struct has prio and tieprio methods,
// both of which return ints.  The list is sorted in increasing order
// on the prio key then secondary sorted in decreasing order on
// the tieprio key.
template <class Ops, class Node> class OpenList <Ops, Node, IntOpenCost> {
public:
	OpenList(void) : fill(0), min(0) { }

	static const char *kind(void) { return "2d bucketed"; }

	void push(Node *n) {
		unsigned long p0 = Ops::prio(n);
 
		if (qs.size() <= p0)
			qs.resize(p0+1);

		if (p0 < min)
			min = p0;

		qs[p0].push(n, Ops::tieprio(n));
		fill++;
	}

	Node *pop(void) {
		for ( ; min < qs.size() && qs[min].empty() ; min++)
			;
		fill--;
		return qs[min].pop();		
	}

	void pre_update(Node*n) {
		if (n->openind < 0)
			return;
		assert ((unsigned long) Ops::prio(n) < qs.size());
		qs[Ops::prio(n)].rm(n, Ops::tieprio(n));
		fill--;
	}

	void post_update(Node *n) {
		assert (n->openind < 0);
		push(n);
	}

	bool empty(void) { return fill == 0; }

	bool mem(Node *n) { return n->openind >= 0; }

	void clear(void) {
		qs.clear();
		min = 0;
	}

private:

	struct Maxq {
		Maxq(void) : fill(0), max(0) { }

		void push(Node *n, unsigned long p) {
			if (bkts.size() <= p)
				bkts.resize(p+1);

			if (p > max)
				max = p;

			n->openind = bkts[p].size();
			bkts[p].push_back(n);
			fill++;
		}

		Node *pop(void) {
			for ( ; max >= 0 && bkts[max].empty(); max--) {
				if (max == 0)
					break;
			}
			Node *n = bkts[max].back();
			bkts[max].pop_back();
			Ops::setind(n, -1);
			fill--;
			return n;
		}

		void rm(Node *n, unsigned long p) {
			assert (p < bkts.size());
			std::vector<Node*> &bkt = bkts[p];

			unsigned int i = n->openind;
			assert (i < bkt.size());

			if (bkt.size() > 1) {
				bkt[i] = bkt[bkt.size() - 1];
				Ops::setind(bkt[i], i);
			}

			bkt.pop_back();
			Ops::setind(n, -1);
			fill--;
		}

		bool empty(void) { return fill == 0; }

		unsigned long fill;
		unsigned int max;
		std::vector< std::vector<Node*> > bkts;
	};

	unsigned long fill;
	unsigned int min;
	std::vector<Maxq> qs;
};

// A Result is returned from a completed search.  It contains
// statistical information about the search along with the
// solution cost and solution path if a goal was found.
template <class D> struct Result : public SearchStats {
	typename D::Cost cost;
	std::vector<typename D::State> path;
	std::vector<typename D::Oper> ops;

	Result(void) : cost(D::InfCost) { }

	// Sets the cost and solution path of the result to that of
	// the given goal node.
	void goal(D &d, SearchNode<D> *goal) {
		cost = goal->g;
		for (SearchNode<D> *n = goal; n; n = n->parent) {
			typename D::State buf, &state = d.unpack(buf, n->packed);
			path.push_back(state);
			if (n->parent)
				ops.push_back(n->op);
		}
	}

	// output writes information to the given file in
	// datafile format.
	void output(FILE *f) {
		dfpair(f, "state size", "%u", sizeof(typename D::State));
		dfpair(f, "packed state size", "%u", sizeof(typename D::PackedState));
		dfpair(f, "final sol cost", "%g", (double) cost);
		dfpair(f, "final sol length", "%lu", (unsigned long) path.size());
		SearchStats::output(f);
	}

	// add accumulates information from another Result
	// in the receiver.  The path infromation is not accumulated.
	void add(Result<D> &other) {
		SearchStats::add(other);
		cost += other.cost;
	}
};

template <class D> class SearchAlgorithm {
public:
	SearchAlgorithm(int argc, const char *argv[]) : lim(argc, argv) { }

	virtual ~SearchAlgorithm() { }

	virtual Result<D> &search(D &, typename D::State &) = 0;

	virtual void reset(void) {
		res = Result<D>();
	}

	virtual void output(FILE *f) {
		lim.output(f);
		res.output(f);
	}

protected:
	bool limit(void) { return lim.reached(res); }
	Result<D> res;
	Limit lim;
};

#endif	// _SEARCH_HPP_
