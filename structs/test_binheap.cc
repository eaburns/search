#include "../incl/utils.hpp"
#include "binheap.hpp"
#include <cstdlib>

struct UIntOps {
	static void setind(unsigned int, int) {}
	static bool pred(unsigned int a, unsigned int b) { return a < b; }
};

unsigned int *ints;

void binheap_push_bench(unsigned long n, double *strt, double *end) {
	Binheap<UIntOps, unsigned int> pq;
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
	Binheap<UIntOps, unsigned int> pq;
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