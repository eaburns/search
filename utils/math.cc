// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "utils.hpp"
#include "safeops.hpp"
#include <boost/cstdint.hpp>
#include <limits>
#include <cmath>
#include <functional>

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

double integrate(std::function<double(double)> getY, double start, double end, double stepsize) {
	double sum = 0;
	double y1 = 0;
	double y2 = getY(start);
	double cur = start;
	double next = start + stepsize;
	for( ; next < end; cur = next, next+=stepsize) {
		
		y1 = y2;
		y2 = getY(next);

		sum += stepsize * ((y1 + y2) / 2);
	}

	y1 = y2;
	y2 = getY(end);
	next = end;
	sum += (next - cur) * ((y1 + y2) / 2);

	return sum;
}

Normal::Normal(double m, double s) : mean(m), stdev(s) {
	pdfcoeff = 1/(stdev*sqrt(2*M_PI));
	cdfcoeff = 1/(sqrt(2*stdev*stdev));
}

double Normal::pdf(double x) const {
	return pdfcoeff * exp(-(x- mean)*(x-mean) / (2*stdev*stdev));
}

double Normal::cdf(double x) const {
	return 0.5 * (1 + erf((x-mean)*cdfcoeff));
}