// Benchmarking for some common basic operations.
//
// Take care when writing these benchmarks that the calls
// to the functions being benchmarked don't actually get
// optimized away.  One way to avoid this is to assign their
// result to some static memory.  See the scratchuints
// function, for example.

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

void log2_double_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);
	double *res = scratchdoubles(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = log2(is[i]);
}

void sqrt_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);
	double *res = scratchdoubles(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = sqrt(is[i]);
}

void atan2_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n*2);
	double *res = scratchdoubles(n);
	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = atan2(is[i], is[i+n]);
}

void sin_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);
	double *res = scratchdoubles(n);

	for (unsigned long i = 0; i < n; i++)
		is[i] *= M_PI * 2;

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = sin(is[i]);
}

void cos_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n);
	double *res = scratchdoubles(n);

	for (unsigned long i = 0; i < n; i++)
		is[i] *= M_PI * 2;

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = cos(is[i]);
}

void pow_bench(unsigned long n, double *strt, double *end) {
	double *is = randdoubles(n*2);
	double *res = scratchdoubles(n);

	for (unsigned long i = n; i < n * 2; i++)
		is[i] *= randgen.real() * 64;

	*strt = walltime();

	for (unsigned long i = 0; i < n; i++)
		res[i] = pow(is[i], is[i+n]);
}

