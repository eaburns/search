#include "../utils/utils.hpp"
#include "closedlist.hpp"
#include <ctime>

struct Ent {
	typedef Ent PackedState;

	static ClosedEntry<Ent, Ent> &closedentry(Ent *n) {
		return n->closedent;
	} 

	static Ent &key(Ent *n) {
		return *n;
	}

	Ent(int i) : vl(i) {}

	Ent() {}

	bool eq(const Ent*, const Ent &o) const {
		return vl == o.vl;
	}

	unsigned int hash(const Ent*) const {
		return vl;
	}

	ClosedEntry<Ent, Ent> closedent;
	unsigned int vl;
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

	for (unsigned int i = 0; i < N; i++) {
		ents[i] = Ent(i);
		closed.add(ents + i);
	}

	for (unsigned int i = 0; i < N; i++) {
		Ent::PackedState ps = ents[i];
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

	for (unsigned int i = 0; i < N; i++) {
		ents[i] = Ent(i);
		closed.add(ents + i);
	}

	unsigned long fill = N;
	for (unsigned int i = 1; i < N; i+=2) {
		Ent e(i);
		if (!closed.remove(e)) {
			testpr("No value mapped to key %u\n", i);
			res = false;
		}
		fill--;	
		if (closed.getFill() != fill) {
			testpr("Expected fill %u, got %u\n", fill, closed.getFill());
			return false;
		}
	}

	for (unsigned int i = 0; i < N; i++) {
		Ent e(i);
		Ent *vlp = closed.find(e);
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

	for (unsigned int i = 0; i < N; i++) {
		ents[i] = Ent(r.bits());
		closed.add(ents + i);
	}

	for (unsigned int i = 0; i < N; i++) {
		Ent *vlp = closed.find(ents[i]);
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

bool closed_iter_test() {
	ClosedList<Ent, Ent, Ent> closed(100);
	Ent ents[N];

	for (unsigned int i = 0; i < N; i++) {
		ents[i] = Ent(i);
		closed.add(ents + i);
	}

	unsigned long cnt = 0;
	for (auto it : closed) {
		if (it->vl != ents[it->vl].vl)
			return false;
		cnt++;
		if (cnt > closed.getFill())
			return false;
	}
	return cnt == closed.getFill();
}