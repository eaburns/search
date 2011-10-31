#ifndef _OPENLIST_HPP_
#define _OPENLIST_HPP_

#include "../structs/intpq.hpp"
#include "../structs/binheap.hpp"

// An A* style open list sorted on minimum f value
// and possibly tie-breaking on high g.
//
// If the Cost type is 'char' then a bucket-based
// priority queue is used, otherwise it's a binary
// heap.
//
// Assumes that your node has a field 'f' and
// either pred(), setind(), getind() or openentry()
// as static functions (the latter is for the bucket-
// based queue, the former are for the binary
// heap).

template <class Ops, class Node, class Cost> class OpenList {
public:
	const char *kind(void) { return "binary heap"; }

	void push(Node *n) { heap.push(n); }

	Node *pop(void) {
		boost::optional< Node* > p = heap.pop();
		if (!p)
			return NULL;
		return *p;
	}

	void pre_update(Node *n) { }

	void post_update(Node *n) { heap.update(n->openind); }

	bool empty(void) { return heap.empty(); }

	bool mem(Node *n) { return Ops::getind(n) != -1; }

	void clear(void) { heap.clear(); }

private:
	struct Heapops {
 		static bool pred(Node *a, Node *b) { return Ops::pred(a, b); }
		static void setind(Node *n, int i) { Ops::setind(n, i); }
		static int getind(Node *n) { return Ops::getind(n); }
	};
	Binheap<Heapops, Node*> heap;
};

template<class Node> struct OpenEntry {
	unsigned int i;
};

template <class Ops, class Node> class OpenList <Ops, Node, char> {
public:
	OpenList(void) : fill(0), min(0) { }

	static const char *kind(void) { return "2d bucketed"; }

	void push(Node *n) {
		unsigned long p0 = Ops::prio(n);
 
		if (qs.size() <= p0)
			qs.resize(p0+1);

		if (p0 < min)
			min = p0;

		qs[p0].push(n, n->g);
		fill++;
	}

	Node *pop(void) {
		for ( ; min < qs.size() && qs[min].empty() ; min++)
			;
		fill--;
		return qs[min].pop();		
	}

	void pre_update(Node*n) {
		assert ((unsigned long) Ops::prio(n) < qs.size());
		qs[Ops::prio(n)].rm(n, n->g);
		fill--;
	}

	void post_update(Node *n) { push(n); }

	bool empty(void) { return fill == 0; }

	bool mem(Node *n) { return Ops::openentry(n).i >= 0; }

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

			Ops::openentry(n).i = bkts[p].size();
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
			Ops::openentry(n).i = -1;
			fill--;
			return n;
		}

		void rm(Node *n, unsigned long p) {
			assert (p < bkts.size());
			std::vector<Node*> &bkt = bkts[p];

			unsigned int i = Ops::openentry(n).i;
			assert (i < bkt.size());

			if (bkt.size() > 1) {
				bkt[i] = bkt[bkt.size() - 1];
				Ops::openentry(bkt[i]).i = i;
			}

			bkt.pop_back();
			Ops::openentry(n).i = -1;
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

#endif	// _OPENLIST_HPP_