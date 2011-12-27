#ifndef _CLOSEDLIST_HPP_
#define _CLOSEDLIST_HPP_

#include "../structs/htable.hpp"
#include <cstdio>

void dfpair(FILE *, const char *key, const char *fmt, ...);	// utils.hpp

enum { FillFact = 0 };

template<class Node, class D> struct ClosedEntry {
	HtableEntry<Node> ent;
};

template<class Ops, class Node, class D> struct ClosedList {
	typedef typename D::PackedState PackedState;

	ClosedList(unsigned long szhint) : tbl(szhint) { }

	void init(D &d) { }

	void clear(void) { tbl.clear(); }

	void add(Node *n) { tbl.add(n); }

	void add(Node *n, unsigned long h) { tbl.add(n, h); }

	Node *find(PackedState &k) { return tbl.find(k); }

	Node *find(PackedState &k, unsigned long h) { return tbl.find(k, h); }

	void prstats(FILE *out, const char *prefix) {
		dfpair(out, "closed list type", "%s", "hash table");
		tbl.prstats(out, prefix);
	}

private:

	struct HtableOps {
		static PackedState &key(Node *n) { return Ops::key(n); }

		static unsigned long hash(PackedState &s) { return Ops::hash(s); }

		static bool eq(PackedState &a, PackedState &b) { return Ops::eq(a, b); }

		static HtableEntry<Node> &entry(Node *n) {
			return Ops::closedentry(n).ent;
		}
	};

	Htable<HtableOps, PackedState&, Node, FillFact> tbl;
};

#endif	// _CLOSEDLIST_HPP_