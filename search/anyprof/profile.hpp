#pragma once

#include <vector>
#include <cassert>
#include <cstdio>

struct Solution {
	double cost, time;
};

typedef std::vector<Solution> SolutionStream;

// AnytimeProfile stores the estimated profile of
// an anytime algorithm, e.g., the probability of
// the algorithm finding a solution of a certain cost
// given the current incumbent cost and an
// amount of time.
class AnytimeProfile {
public:

	// This constructor creates an empty profile with
	// the specified number of bins for cost and time.
	AnytimeProfile(unsigned int costbins, unsigned int timebins,
		const std::vector<SolutionStream>&);

	// This constructor reads a profile from the given FILE*.
	AnytimeProfile(FILE*);

	~AnytimeProfile();

	// save saves the profile to disk.
	void save(FILE*) const;

	// qtcount returns the count for the ith cost
	// and jth time step.
	unsigned int qtcount(int i, int j) const {
		return qtcounts[qtindex(i, j)];
	}

	// prob returns the estimated probability of getting
	// a solution of cost qj after dt time passes when the
	// incumbent solution has a cost of qi: P(qj | qi, Δt).
	double prob(double qj, double qi, double dt) const;

	// ncost is the number of cost bins, and ntime
	// is the number of time bins.
	unsigned int ncost, ntime;

	// mincost and maxcost define the range of cost
	// values used to train the profile.
	double mincost, maxcost;

	// mintime and maxtime define the range of time
	// values used to train the profile.
	double mintime, maxtime;

private:

	void seesolutions(const std::vector<SolutionStream>&);
	void initbins(unsigned int, unsigned int);
	void getextents(const std::vector<SolutionStream>&);
	void countsolutions(const std::vector<SolutionStream>&);
	void mkprobs();

	// qtindex returns an index into the qtcounts array.
	unsigned int qtindex(int qi, int dt) const {
		return qi*ntime + dt;
	}

	// qqtindex returns an index into the qqtcounts array.
	unsigned int qqtindex(int qj, int qi, int dt) const {
		return qj*ncost*ntime + qi*ntime + dt;
	}

	// costbin returns the bin number into which the
	// given cost value should reside.
	unsigned int costbin(double c) const {
		assert (c >= mincost);
		assert (c <= maxcost);

		if (c == maxcost)
			return ncost-1;
		return (c-mincost)/ncost;
	}

	// timebin returns the bin into which the given
	// Δt value should reside.
	unsigned int timebin(double dt) const {
		assert (dt >= 0);
		assert (dt <= maxtime-mintime);

		if (dt == maxtime-mintime)
			return ntime-1;
		return dt/ntime;
	}

	// qtcounts is an array of the number of solutions
	// for each cost, time pair.
	unsigned int *qtcounts;

	// qqtcounts is an array of the number of solutions
	// for each cost, cost, time triple.
	unsigned int *qqtcounts;

	// qqtprobs is an array of the probability estimate
	// for each cost, cost, time triple.
	double *qqtprobs;
};


class AnytimeMonitor {
public:

	// AnytimeMonitor creates an anytime monitoring
	// policy given a profile and the cost and time
	// weights that define the user's utility function.
	AnytimeMonitor(const AnytimeProfile&, double wf, double wt);

	// deltat returns the time (in seconds) between which
	// the monitor should be checked.
	double deltat() const {
		return (profile.maxtime - profile.mintime) / profile.ntime;
	}

private:

	double wf, wt;

	const AnytimeProfile &profile;
};