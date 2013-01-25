#include "safeops.hpp"
#include <boost/cstdint.hpp>
#include <limits>
#include <cmath>

void fatal(const char*, ...);

unsigned int ilog2(boost::uint32_t v) {
	if (!v)
		return 0;

	unsigned int lg = 0;

	if (v & 0xFFFF0000) {
		lg += 16;
		v >>= 16;
	}
	if (v & 0xFF00) {
		lg += 8;
		v >>= 8;
	}
	if (v & 0xF0) {
		lg += 4;
		v >>= 4;
	}
	if (v & 0xC) {
		lg += 2;
		v >>= 2;
	}
	if (v & 0x2) {
		lg += 1;
		v >>= 1;
	}
	return lg + 1;
}

unsigned long ipow(unsigned int b, unsigned int e) {
	unsigned long r = 1;
	for (unsigned int i = 0; i < e; i++)
		r *= (unsigned long) b;
	return r;
}

unsigned long fallfact(unsigned int x, unsigned int n) {
	unsigned long f = x;
	for (unsigned int i = 1; i < n; i++)
		f *= (x - i);
	return f;
}

double normcdf(double mu, double sigma, double x) {
	return 0.5 * (1 + erf((x-mu)/sqrt(2*sigma*sigma)));
}

double phi(double x) {
	static const double sqrt2 = sqrt(2);
	return 0.5 * (1 + erf(x/sqrt2));
}