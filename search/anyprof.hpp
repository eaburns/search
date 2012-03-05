#include <vector>
#include <cassert>
#include <cstdio>
#include <utility>
#include <cmath>

// AnyProf holds an anytime algorithm's performance profile.
struct AnyProf {

	AnyProf(void) { }

	AnyProf(unsigned int cbins, double cmax, unsigned int tbins, double tmax);

	void read(FILE*);

	void write(FILE*) const;

	unsigned int cbins, tbins;
	double cmax, tmax, cwidth, twidth;

	// indexed on cost, delta time, and cost.
	std::vector< std::vector< std::vector<double> > > bins;
};

// MonPolicy is an anytime algorithm monitoring policy
// that is constructed as in "Monitoring and control of
// anytime algorithms: A dynamic programming approach,"
// by Eric A. Hansen and Shlomo Zilberstein, 2001.
struct MonPolicy {

	MonPolicy(void) { }

	MonPolicy(const AnyProf&, double wcost, double wtime);

	// next is the monitoring function, it returns the time
	// at which the next monitoring should take place and
	// a boolean value that is true if the algorithm should
	// halt instead of monitoring after this time has elapsed.
	std::pair<double, bool> next(double cost, double time) {
		unsigned int ci = cost / prof.cwidth;
		unsigned int ti = time / prof.twidth;

		if (ci >= prof.cbins)
			ci = prof.cbins-1;

		if (ti >= prof.tbins)
			return std::pair<double,bool>(0, true);

		return std::pair<double,bool>(ents[ci][ti].delta, ents[ci][ti].stop);
	}

	AnyProf prof;
	double wcost, wtime;

	struct Entry {
		Entry(void) : set(false) { }

		Entry(double v, double d, bool s) :
			set(true), value(v), delta(d), stop(s) { }

		bool set;
		double value;
		double delta;
		bool stop;
	};

	std::vector< std::vector<Entry> > ents;
};
