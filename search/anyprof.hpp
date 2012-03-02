#include <vector>
#include <cassert>
#include <cstdio>

// Imp holds information about a solution improvement
// such as the initial cost, the new cost and the time delta
// between them.
struct Imp {
	Imp(double _q0, double _t0, double _q1, double _t1) :
		q0(_q0), t0(_t0), q1(_q1), t1(_t1) { }
	Imp(void) { }
	double q0, t0, q1, t1;
};

// AnyProf holds an anytime algorithm's performance profile.
struct AnyProf {

	AnyProf(void) { }

	AnyProf(const std::vector<Imp>&, int, int);

	// pr returns the probability of an improvement
	// from c0 to c1 given dt time.
	double pr(double q0, double q1, double dt) const {
		unsigned int i = index(q0, q1, dt);
		return i >= prof.size() ? 0 : prof[i];
	}

	// write writes the profile to the given file stream.
	void write(FILE*) const;

	// read reads the profile from the given file stream.
	// The current profile is clobbered by the one read in.
	void read(FILE*);

private:

	void maxes(const std::vector<Imp>&);

	// index returns the index of the probability entry
	// for the change from c0 to c1 in time dt.
	unsigned int index(double q0, double q1, double dt) const {
		double q0bin = q0 / qwidth;
		double dtbin = dt / twidth;
		double q1bin = q1 / qwidth;
		return q0bin*dtbins*qbins + dtbin*qbins + q1bin;
	}

	friend struct MonPolicy;

	double qmax, dtmax, tmax;	// maximum cost, delta time and time
	double qwidth, twidth;	// width of each cost and delta time bin
	int qbins, dtbins;	// number of cost and delta time bins

	std::vector<double> prof;
};

// MonPolicy is an anytime algorithm monitoring policy
// that is constructed as in "Monitoring and control of
// anytime algorithms: A dynamic programming approach,"
// by Eric A. Hansen and Shlomo Zilberstein, 2001.
struct MonPolicy {

	MonPolicy(const AnyProf&, double, double);

private:

	double usum(double q0, double dt, double t1) const;

	double vsum(double q0, double dt, double t1) const;

	unsigned int index(double q, double t) const {
		int qbin = q / prof.qwidth;
		int tbin = t / prof.twidth;
		return qbin*tbins + tbin;
	}

	std::vector<double> v;
	std::vector<bool> stop;
	std::vector<double> delta;

	double wq, wt;
	AnyProf prof;
	int qbins, tbins;
};