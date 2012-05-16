#include "plat2d.hpp"
#include "../search/closedlist.hpp"

template<class Node> struct ClosedEntry<Node, Plat2d> { };

template<class Ops, class Node> struct ClosedList<Ops, Node, Plat2d> {
	typedef Plat2d::PackedState PackedState;

	ClosedList(unsigned long) { }

	~ClosedList() { }

	void init(Plat2d&) { }

	void clear() { }

	void add(Node*) { }

	void add(Node*, unsigned long) { }

	Node *find(PackedState&) { return NULL; }

	Node *find(PackedState&, unsigned long) { return NULL; }

	void prstats(FILE *out, const char *prefix) {
		dfpair(out, "closed list type", "%s", "none");
	}
};