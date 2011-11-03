#include "gridpath.hpp"
#include "../search/closedlist.hpp"
#include <cassert>

template<class Node> struct ClosedEntry<Node, GridPath> { };

template<class Ops, class Node> struct ClosedList<Ops, Node, GridPath> {
	typedef GridPath::PackedState PackedState;

	ClosedList(unsigned long szhint) : len(0), cap(szhint) {
		nodes = new Node*[cap];
	}

	~ClosedList(void) { delete[] nodes; }

	void init(GridPath &d) {
		len = d.width() * d.height();
		if (cap >= len)
			return;

		cap = len;

		delete[] nodes;
		nodes = new Node*[cap];

		for (unsigned int i = 0; i < cap; i++)
			nodes[i] = NULL;
	}

	void clear(void) {
		for (unsigned int i = 0; i < cap; i++)
			nodes[i] = NULL;
	}

	void add(Node *n) { add(n, Ops::key(n).loc); }

	void add(Node *n, unsigned long h) {
		assert(h < len);
		nodes[h] = n;
	}

	Node *find(PackedState &k) { find(k, k.loc); }

	Node *find(PackedState &k, unsigned long h) {
		assert(h < len);
		return nodes[h];
	}

	void prstats(FILE *out, const char *prefix) {
		dfpair(out, "closed list type", "%s", "array");
	}

private:

	unsigned long len, cap;
	Node **nodes;
};