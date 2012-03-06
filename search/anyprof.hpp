#include <vector>
#include <cassert>
#include <cstdio>
#include <utility>
#include <cmath>

void dfpair(FILE *, const char *key, const char *fmt, ...);

// AnyProf holds an anytime algorithm's performance profile.
struct AnyProf {

	AnyProf(void) : cbins(0), tbins(0) { }

	AnyProf(unsigned int cbins, double cmax, unsigned int tbins, double tmax);

	// read reads the profile from the given file stream,
	// stomping any previous information stored in the
	// current profile.
	void read(FILE*);

	// write writes the profile to the given file stream.
	void write(FILE*) const;

	// output outputs information about the profile in
	// datafile format.
	void output(FILE *f) const {
		dfpair(stdout, "profile cost max", "%g", cmax);
		dfpair(stdout, "profile time max", "%g", tmax);
		dfpair(stdout, "profile cost bins", "%u", cbins);
		dfpair(stdout, "profile time bins", "%u", tbins);
		dfpair(stdout, "profile cost width", "%g", cwidth);
		dfpair(stdout, "profile time width", "%g", twidth);
	}

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

	// output outputs information about the monitor.
	void output(FILE *f) const { prof.output(f); }

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

		assert (ents[ci][ti].set);

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
