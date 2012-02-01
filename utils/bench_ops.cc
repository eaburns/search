// Benchmarking for some common basic operations.
//
// Take care when writing these benchmarks that the calls
// to the functions being benchmarked don't actually get
// optimized away.  One way to avoid this is to assign their
// result to some static memory.

#include "utils.hpp"
#include <cmath>

void malloc_16_free_bench(unsigned long n, double *strt, double *end) {
	for (unsigned long i = 0; i < n; i++)
		free(malloc(16));
}

void malloc_32_free_bench(unsigned long n, double *strt, double *end) {
	for (unsigned long i = 0; i < n; i++)
		free(malloc(32));
}

void malloc_128_free_bench(unsigned long n, double *strt, double *end) {
	for (unsigned long i = 0; i < n; i++)
		free(malloc(128));
}

void rand_bits_bench(unsigned long n, double *strt, double *end) {
	unsigned int *res = randuints(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = randgen.bits();
}

void rand_real_bench(unsigned long n, double *strt, double *end) {
	double *res = randdoubles(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = randgen.real();
}

void ilog2_bench(unsigned long n, double *strt, double *end) {
	unsigned int *is = randuints(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		is[i] = ilog2(is[i]);
}

void log2_int_bench(unsigned long n, double *strt, double *end) {
	unsigned int *is = randuints(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		is[i] = log2(is[i]);
}

void log2_double_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		is[i] = log2(is[i]);
}

void sqrt_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		is[i] = sqrt(is[i]);
}

void atan2_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		is[i] = atan2(is[i], is[i+1]);
}

void sin_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);

	for (unsigned long i = 0; i < n; i++)
		is[i] *= M_PI * 2;

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		is[i] = sin(is[i]);
}

void cos_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);

	for (unsigned long i = 0; i < n; i++)
		is[i] *= M_PI * 2;

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		is[i] = cos(is[i]);
}

void pow_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);

	for (unsigned long i = n; i < n; i++)
		is[i] *= randgen.real() * 64;

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		is[i] = pow(is[i], is[i+1]);
}

