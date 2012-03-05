#include <vector>
#include <cassert>
#include <cstdio>
#include <utility>
#include <cmath>

// AnyProf holds an anytime algorithm's performance profile.
struct AnyProf {

	AnyProf(void) { }

	AnyProf(unsigned int cbins, double cmax, unsigned int tbins, double tmax);

	void read(FILE*) { }

	void write(FILE*) const { }

	unsigned int cbins, tbins;
	double cmax, tmax, cwidth, twidth;
	std::vector< std::vector< std::vector<double> > > bins;
};

// MonPolicy is an anytime algorithm monitoring policy
// that is constructed as in "Monitoring and control of
// anytime algorithms: A dynamic programming approach,"
// by Eric A. Hansen and Shlomo Zilberstein, 2001.
struct MonPolicy {

	MonPolicy(void) { }

	MonPolicy(const AnyProf&, double, double) { }

	// next is the monitoring function, it returns the time
	// at which the next monitoring should take place and
	// a boolean value that is true if the algorithm should
	// halt instead of monitoring after this time has elapsed.
	std::pair<double, bool> next(double q, double t) {
			return std::pair<double, bool>(0, true);
	}

	AnyProf prof;
};