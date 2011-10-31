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
	IntpqEntry<Node> node;
};

template <class Ops, class Node> class OpenList <Ops, Node, char> {
public:
	static const char *kind(void) { return "bucketed"; }

	void push(Node *n) { pq.push(n, Ops::prio(n)); }

	Node *pop(void) { return pq.pop(); }

	void pre_update(Node*n) { pq.rm(n, Ops::prio(n)); }

	void post_update(Node *n) { push(n); }

	bool empty(void) { return pq.empty(); }

	bool mem(Node *n) { return pq.mem(n); }

	void clear(void) { pq.clear(); }

	typedef IntpqEntry<Node> OpenEntry;

private:
	struct Intpqops {
		static IntpqEntry<Node> &entry(Node *n) {
			return Ops::openentry(n).node;
		}
	};
	Intpq<Intpqops, Node> pq;
};

#endif	// _OPENLIST_HPP_