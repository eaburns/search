#include "../incl/utils.hpp"
#include "intpq.hpp"
#include <cstdlib>

struct Elm {
	Elm *prev, *nxt;
	unsigned int vl;
};

struct Ops {
	static Elm **nxt(Elm *e) { return &e->nxt; }
	static Elm **prev(Elm *e) { return &e->prev; }
};

enum { N = 1000 };

bool intpq_push_test(void) {
	bool res = true;
	Intpq<Ops, Elm> pq;
	Elm elms[N];

	for (unsigned int i = 0; i < N; i++) {
		elms[i].vl = rand() % 1000;
		pq.push(&elms[i], elms[i].vl);
		if (pq.fill != (unsigned long) i+1) {
			testpr("Expected fill of %u got %lu\n", i+1, pq.fill);
			res = false;
		}
	}
 
	return res;
}

bool intpq_pop_test(void) {
	bool res = true;
	Intpq<Ops, Elm> pq;
	Elm elms[N];

	for (int i = 0; i < N; i++) {
		elms[i].vl = rand() % 1000;
		pq.push(&elms[i], elms[i].vl);
	}

	Elm *e = pq.pop();
	if (!e) {
		testpr("Empty pop");
		res = false;
	}
	int min = e->vl;

	for (unsigned int i = 1; i < N; i++) {
		e = pq.pop();
		if (!e) {
			testpr("Empty pop");
			res = false;
		}

		int m = e->vl;
		if (pq.fill != (unsigned long) N - (i+1)) {
			testpr("Expected fill of %u got %lu\n", N-(i+1), pq.fill);
			res = false;
		}

		if (m < min) {
			testpr("%d came out after %d\n", m, min);
			res = false;
		}
		min = m;
	}
 
	return res;
}

Elm *elms;

void intpq_push_bench(unsigned long n, double *strt, double *end) {
	Intpq<Ops, Elm> pq;
	elms = new Elm[n];

	for (unsigned long i = 0; i < n; i++)
		elms[i].vl = rand() % 1000;

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		pq.push(&elms[i], elms[i].vl);

	*end = walltime();
 
	delete[] elms;
}

void intpq_pop_bench(unsigned long n, double *strt, double *end) {
	Intpq<Ops, Elm> pq;
	elms = new Elm[n];

	for (unsigned long i = 0; i < n; i++) {
		elms[i].vl = rand() % 1000;
		pq.push(&elms[i], elms[i].vl);
	}


	*strt = walltime();
 
	for (unsigned long i = 0; i < n; i++)
		pq.pop();

	*end = walltime();
 
	delete[] elms;
}