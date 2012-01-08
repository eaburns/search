#include "../utils/utils.hpp"
#include "htable.hpp"
#include <ctime>

struct Ent {
	unsigned int vl;
	HtableEntry<Ent> htent;
	Ent(int i) : vl(i) {}
	Ent() {}
};

typedef unsigned int Key;

struct Ops {
	static Key key(Ent *e) { return e->vl; }
	static unsigned long hash(Key k) { return k; }
	static HtableEntry<Ent> &entry(Ent *e) { return e->htent; }
	static bool eq(Key a, Key b) { return a == b; }
};

enum { N = 1000 };

bool htable_add_test(void) {
	bool res = true;
	Htable<Ops, Key, Ent> ht;
	Ent ents[N];

	if (ht.fill != 0) {
		testpr("Hash table fill is not initialized to zero\n");
		res = false;
	}

	for (unsigned int i = 0; i < N; i++) {
		ents[i] = Ent(i);
		ht.add(ents + i);
		if (ht.fill != i+1) {
			testpr("Hash table fill is not %u after %u adds\n", i+1, i+1);
			res = false;
		}
	}

	return res;
}

bool htable_find_test(void) {
	bool res = true;
	Htable<Ops, Key, Ent> ht;
	Ent ents[N];

	for (unsigned int i = 0; i < 100; i++) {
		ents[i] = Ent(i);
		ht.add(ents + i);
	}

	for (unsigned int i = 0; i < 100; i++) {
		Ent *vlp = ht.find(i);
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

bool htable_find_rand_test(void) {
	bool res = true;
	Htable<Ops, Key, Ent> ht;
	Rand r(time(NULL));
	Ent ents[N];

	for (unsigned int i = 0; i < 100; i++) {
		ents[i] = Ent(r.bits());
		ht.add(ents + i);
	}

	for (unsigned int i = 0; i < 100; i++) {
		Ent *vlp = ht.find(ents[i].vl);
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

Ent *ents;
unsigned int *inds;

void htable_add_bench(unsigned long n, double *strt, double *end) {
	Htable<Ops, Key, Ent> ht;
	Rand r(time(NULL));

	ents = new Ent[n];
	for (unsigned long i = 0; i < n; i++)
		ents[i] = Ent(r.bits());

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		ht.add(&ents[i]);

	*end = walltime();

	delete[] ents;
}

void htable_find_bench(unsigned long n, double *strt, double *end) {
	Htable<Ops, Key, Ent> ht;
	Rand r(time(NULL));

	ents = new Ent[n];
	inds = new unsigned int[n];
	for (unsigned long i = 0; i < n; i++) {
		ents[i] = Ent(r.bits());
		ht.add(&ents[i]);
		inds[i] = r.bits() % n;
	}

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		ht.find(ents[inds[i]].vl);

	*end = walltime();

	delete[] inds;
	delete[] ents;
}
