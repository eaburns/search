// Benchmarking for some common basic operations.
//
// Take care when writing these benchmarks that the calls
// to the functions being benchmarked don't actually get
// optimized away.  One way to avoid this is to assign their
// result to some static memory.  See the scratchuints
// function, for example.

#include "utils.hpp"
#include <cmath>

void rand_bits_bench(unsigned long n, double *strt, double *end) {
	unsigned int *res = scratchuints(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = randgen.bits();
}

void rand_real_bench(unsigned long n, double *strt, double *end) {
	double *res = scratchdoubles(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = randgen.real();
}

void ilog2_bench(unsigned long n, double *strt, double *end) {
	unsigned int *is = randuints(n);
	unsigned int *res = scratchuints(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = ilog2(is[i]);
}

void log2_int_bench(unsigned long n, double *strt, double *end) {
	unsigned int *is = randuints(n);
	unsigned int *res = scratchuints(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = log2(is[i]);
}