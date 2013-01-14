#include "../utils/utils.hpp"
#include "closedlist.hpp"
#include <ctime>

struct Ent {
	typedef unsigned int PackedState;

	static unsigned int hash(PackedState &ps) { return ps; }

	static ClosedEntry<Ent, Ent> &closedentry(Ent *n) { return n->closedent; }

	static PackedState &key(Ent *n) { return n->packed; }

	Ent(int i) : vl(i), packed(i) {}

	Ent() {}

	PackedState pack() { return vl; }

	ClosedEntry<Ent, Ent> closedent;
	unsigned int vl;
	PackedState packed;
};

typedef unsigned int Key;

enum { N = 1000 };

bool closedlist_add_test() {
	bool res = true;
	ClosedList<Ent, Ent, Ent> closed(100);
	Ent ents[N];

	if (closed.getFill() != 0) {
		testpr("Hash table fill is not initialized to zero\n");
		res = false;
	}

	for (unsigned int i = 0; i < N; i++) {
		ents[i] = Ent(i);
		closed.add(ents + i);
		if (closed.getFill() != i+1) {
			testpr("Closed List fill is not %u after %u adds\n", i+1, i+1);
			res = false;
		}
	}

	return res;
}

bool closedlist_find_test() {
	bool res = true;
	ClosedList<Ent, Ent, Ent> closed(100);
	Ent ents[N];

	for (unsigned int i = 0; i < 100; i++) {
		ents[i] = Ent(i);
		closed.add(ents + i);
	}

	for (unsigned int i = 0; i < 100; i++) {
		Ent::PackedState ps = ents[i].pack();
		Ent *vlp = closed.find(ps);
		if (!vlp) {
			testpr("No value mapped to key %u\n", i);
			res = false;
		} else if (vlp->vl != i) {
			testpr("Found value %u mapped to key %u\n", vlp->vl, i);
			res = false;
		}
	}

	return res;
}

bool closedlist_rm_test() {
	bool res = true;
	ClosedList<Ent, Ent, Ent> closed(100);
	Ent ents[N];

	unsigned int nvals = 100;

	for (unsigned int i = 0; i < nvals; i++) {
		ents[i] = Ent(i);
		closed.add(ents + i);
	}

	unsigned long fill = nvals;
	for (unsigned int i = 1; i < nvals; i+=2) {
		if (!closed.remove(i)) {
			testpr("No value mapped to key %u\n", i);
			res = false;
		}
		fill--;	
		if (closed.getFill() != fill) {
			testpr("Expected fill %u, got %u\n", fill, closed.getFill());
			return false;
		}
	}

	for (unsigned int i = 0; i < nvals; i++) {
		Ent *vlp = closed.find(i);
		if (!vlp && i % 2 == 0) {
			testpr("No value mapped to even key %u\n", i);
			res = false;
		}
		if (vlp && i % 2 != 0) {
			testpr("Odd key %u was never removed\n", i);
			res = false;
		}
	}

	return res;
}

bool closedlist_find_rand_test() {
	bool res = true;
	ClosedList<Ent, Ent, Ent> closed(100);
	Rand r(time(NULL));
	Ent ents[N];

	for (unsigned int i = 0; i < 100; i++) {
		ents[i] = Ent(r.bits());
		closed.add(ents + i);
	}

	for (unsigned int i = 0; i < 100; i++) {
		Ent *vlp = closed.find(ents[i].vl);
		if (!vlp) {
			testpr("No value mapped to key %u\n", i);
			res = false;
		} else if (vlp != ents + i) {
			testpr("Found value wrong entry\n");
			res = false;
		}
	}

	return res;
}

bool closedlist_iter_test() {
	bool res = true;

	ClosedList<Ent, Ent, Ent> closed(1000);
	Ent ents[N];

	if (closed.getFill() != 0) {
		testpr("Hash table fill is not initialized to zero\n");
		res = false;
	}

	for (unsigned int i = 0; i < N; i++) {
		ents[i] = Ent(i);
		closed.add(ents + i);
		if (closed.getFill() != i+1) {
			testpr("Closed List fill is not %u after %u adds\n", i+1, i+1);
			res = false;
		}
	}
	ClosedList<Ent, Ent, Ent>::iterator iter = closed.begin();

	for(unsigned int i = 0; i < N; i++) {
		Ent* e = iter.next();
		if(e->vl != i)
			res = false;
	}

	if(iter.next() != NULL)
		res = false;

	return res;
}

bool closedlist_iter_test2() {
	bool res = true;

	ClosedList<Ent, Ent, Ent> closed(10);
	Ent ents[N];

	if (closed.getFill() != 0) {
		testpr("Hash table fill is not initialized to zero\n");
		res = false;
	}

	for (unsigned int i = 0; i < N; i++) {
		ents[i] = Ent(i);
		closed.add(ents + i);
		if (closed.getFill() != i+1) {
			testpr("Closed List fill is not %u after %u adds\n", i+1, i+1);
			res = false;
		}
	}

	unsigned int count = 0;

	for(ClosedList<Ent, Ent, Ent>::iterator iter = closed.begin();
		iter.next() != NULL; count++) {}

	if(count != N)
		res = false;

	return res;
}