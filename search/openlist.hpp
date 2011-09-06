#include "../structs/intpq.hpp"
#include "../structs/binheap.hpp"

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

private:
	struct Heapops {
 		static bool pred(Node *a, Node *b) { return Ops::pred(a, b); }
		static void setind(Node *n, int i) { Ops::setind(n, i); }
		static int getind(Node *n) { return Ops::getind(n); }
	};
	Binheap< Heapops, Node* > heap;
};

template <class Ops, class Node> class OpenList <Ops, Node, char> {
public:
	static const char *kind(void) { return "bucketed"; }

	void push(Node *n) { pq.push(n, Ops::prio(n)); }

	Node *pop(void) { return pq.pop(); }

	void pre_update(Node*n) { pq.rm(n); }

	void post_update(Node *n) { push(n); }

	bool empty(void) { return pq.empty(); }

	bool mem(Node *n) { return pq.mem(n); }

private:
	struct Intpqops {
		static Node **nxt(Node *n) { return Ops::nxt(n); }
		static Node **prev(Node *n) { return Ops::prev(n); }
	};
	Intpq< Intpqops, Node > pq;
};

template <class Ops, class Node>
class OpenList <Ops, Node, short> : public OpenList <Ops, Node, char > {};

template <class Ops, class Node>
class OpenList <Ops, Node, int> : public OpenList <Ops, Node, char > {};

template <class Ops, class Node>
class OpenList <Ops, Node, long> : public OpenList <Ops, Node, char > {};