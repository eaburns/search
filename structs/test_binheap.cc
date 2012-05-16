#include "../utils/utils.hpp"
#include "binheap.hpp"
#include <cstdlib>

struct UIntOps {
	static void setind(unsigned int, int) {}
	static bool pred(unsigned int a, unsigned int b) { return a < b; }
};

enum { N = 1000 };

bool binheap_push_test() {
	bool res = true;
	BinHeap<UIntOps, unsigned int> pq;
	unsigned int ints[N];

	for (unsigned int i = 0; i < N; i++) {
		ints[i] = rand() % 100;
		pq.push(ints[i]);
		if (pq.heap.size() != (unsigned long) i+1) {
			testpr("Expected fill of %u got %lu\n", i+1, pq.heap.size());
			res = false;
		}
	}
 
	return res;
}

bool binheap_pop_test() {
	bool res = true;
	BinHeap<UIntOps, unsigned int> pq;
	unsigned int ints[N];

	for (int i = 0; i < N; i++) {
		ints[i] = rand() % 100;
		pq.push(ints[i]);
	}

	boost::optional<unsigned int> e = pq.pop();
	if (!e) {
		testpr("Empty pop");
		res = false;
	}
	int min = *e;

	for (unsigned int i = 1; i < N; i++) {
		e = pq.pop();
		if (!e) {
			testpr("Empty pop");
			res = false;
		}

		int m = *e;
		if (pq.heap.size() != (unsigned long) N - (i+1)) {
			testpr("Expected fill of %u got %lu\n", N-(i+1), pq.heap.size());
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

unsigned int *ints;

void binheap_push_bench(unsigned long n, double *strt, double *end) {
	BinHeap<UIntOps, unsigned int> pq;
	ints = new unsigned int[n];

	for (unsigned long i = 0; i < n; i++)
		ints[i] = rand() % 1000;

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		pq.push(ints[i]);

	*end = walltime();
 
	delete[] ints;
}

void binheap_pop_bench(unsigned long n, double *strt, double *end) {
	BinHeap<UIntOps, unsigned int> pq;
	ints = new unsigned int[n];

	for (unsigned long i = 0; i < n; i++) {
		ints[i] = rand() % 1000;
		pq.push(ints[i]);
	}


	*strt = walltime();
 
	for (unsigned long i = 0; i < n; i++)
		pq.pop();

	*end = walltime();
 
	delete[] ints;
}