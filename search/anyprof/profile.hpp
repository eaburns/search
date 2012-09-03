#pragma once

#include <vector>
#include <cassert>
#include <cstdio>
#include <string>

// AnytimeProfile stores the estimated profile of
// an anytime algorithm, e.g., the probability of
// the algorithm finding a solution of a certain cost
// given the current incumbent cost and an
// amount of time.
class AnytimeProfile {
public:	
	
	struct Solution {
		double cost, time;
	};
	
	typedef std::vector<Solution> SolutionStream;

	AnytimeProfile() { }

	// This constructor creates an empty profile with
	// the specified number of bins for cost and time.
	AnytimeProfile(unsigned int costbins, unsigned int timebins,
		const std::vector<SolutionStream>&);

	// This constructor reads the proflie from the file
	// specified by the given path.
	AnytimeProfile(const std::string&);

	// This constructor reads a profile from the given FILE*.
	AnytimeProfile(FILE*);

	// save saves the profile to disk.
	void save(FILE*) const;

	// qtcount returns the count for the ith cost
	// and jth time step.
	unsigned int qtcount(int i, int j) const {
		return qtcounts[qtindex(i, j)];
	}

	// binprob returns the estimated probability of getting
	// a solution of cost qj after dt time passes when the
	// incumbent solution has a cost of qi: P(qj | qi, Δt).
	double binprob(unsigned int q2, unsigned int q1, unsigned int dt) const {	
		if (q2 >= ncost || q1 >= ncost || dt >= ntime)
			return 0.0;
		return qqtprobs[qqtindex(q2, q1, dt)];
	}

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

	void read(FILE*);
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
		double width = (maxcost-mincost)/ncost;
		return (c-mincost)/width;
	}

	// timebin returns the bin into which the given
	// Δt value should reside.
	unsigned int timebin(double dt) const {
		assert (dt >= 0);
		assert (dt <= maxtime-mintime);

		if (dt == maxtime-mintime)
			return ntime-1;
		double width = (maxtime-mintime)/ntime;
		return dt/width;
	}

	// qtcounts is an array of the number of solutions
	// for each cost, time pair.
	std::vector<unsigned int> qtcounts;

	// qqtcounts is an array of the number of solutions
	// for each cost, cost, time triple.
	std::vector<unsigned int> qqtcounts;

	// qqtprobs is an array of the probability estimate
	// for each cost, cost, time triple.
	std::vector<double>qqtprobs;
};


class AnytimeMonitor {
public:

	AnytimeMonitor() { }

	// AnytimeMonitor creates an anytime monitoring
	// policy given a profile and the cost and time
	// weights that define the user's utility function.
	AnytimeMonitor(const AnytimeProfile&, double wf, double wt);
	// deltat returns the time (in seconds) between which
	// the monitor should be checked.
	double deltat() const {
		return (prof.maxtime - prof.mintime) / prof.ntime;
	}

	// stop returns true if the policy is to stop with
	// the given solution cost at the given time. 
	bool stop(double cost, double time) const {
		if (cost < prof.mincost || time < prof.mintime)
			return false;
		if (cost >= prof.maxcost || time >= prof.maxtime)
			return true;

		unsigned int q = (cost - prof.mincost) / qwidth;
		assert (q < prof.ncost);

		unsigned int t = (time - prof.mintime) / twidth;
		assert (t < prof.ntime);

		return policy[index(q, t)];
	}

private:

	// bincost is the cost of the qth cost bin.
	double bincost(unsigned int q) const {
		assert (q < prof.ncost);
		return prof.mincost + (q*qwidth) + qwidth/2;
	}

	// bintime is the time of the tth time bin.
	double bintime(unsigned int t) const {
		assert (t < prof.ntime);
		return prof.mintime + (t*twidth) + twidth/2;
	}

	// binutil returns the utility of q, t bin.
	double binutil(unsigned int q, unsigned int t) const {
		return -(wf*bincost(q) + wt*bintime(t));
	}

	// index returns the 2-dimensional array index
	// for the given cost and time pair.
	unsigned int index(unsigned int q, unsigned int t) const {
		assert (q < prof.ncost);
		assert (t < prof.ntime);
		return q*prof.ntime + t;
	}

	AnytimeProfile prof;

	// wf and wt give the user's utility function as a linear
	// combination of cost and time: u = wf*cost + wt*time.
	double wf, wt;

	// qwidth and twidth are the width of the cost and
	// time bins.
	double qwidth, twidth;

	// value is the value of the cost and time pairs.
	std::vector<double> value;

	// policy is the stopping policy.
	std::vector<bool> policy;

	// seen is for debugging, it is set to true if we have
	// set the value and policy for the given cost, time
	// pair.
	std::vector<bool> seen;
};