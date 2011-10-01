#ifndef _SEARCH_HPP_
#define _SEARCH_HPP_

#include <cstdio>
#include <vector>

double walltime(void);
double cputime(void);
void dfpair(FILE *f, const char *key, const char *fmt, ...);

struct SearchStats {
	double wallstrt, cpustrt;
	double wallend, cpuend;
	unsigned long expd, gend, reopnd, dups;

	SearchStats(void);
	void start(void);
	void finish(void);
	void output(FILE*);

	// After adding, wallstrt and cpustrt are both 0 and the
	// respective end times are such that the total time
	// is equal to the sum of the search times of this and
	// other.
	void add(SearchStats &other) {
		expd += other.expd;
		gend += other.gend;
		reopnd += other.reopnd;
		dups += other.dups;

		double wt = wallend - wallstrt;
		double ct = cpuend - cpustrt;
		wallstrt = cpustrt = 0;
		wallend = wt + (other.wallend - other.wallstrt);
		cpuend = ct + (other.cpuend - other.cpustrt);
	}
};

template <class D> struct Result : public SearchStats {
	typename D::Cost cost;
	std::vector<typename D::State> path;

	Result(void) : cost(D::InfCost) { }

	void output(FILE *f) {
		dfpair(f, "state size", "%u", sizeof(typename D::State));
		dfpair(f, "packed state size", "%u", sizeof(typename D::PackedState));
		dfpair(f, "final sol cost", "%g", (double) cost);
		dfpair(f, "final sol length", "%lu", (unsigned long) path.size());
		SearchStats::output(f);
	}

	void add(Result<D> &other) {
		SearchStats::add(other);
		cost += other.cost;
	}
};

struct Limit {
	unsigned long expd, gend;

	bool reached(SearchStats &r) {
		return (expd > 0 && r.expd >= expd)
			|| (gend > 0 && r.gend >= gend);
	}

	Limit(void);
	Limit(int, char*[]);
	void output(FILE*);
};

template <class D> class Search {
public:
	Search(int argc, char *argv[]) : lim(argc, argv) { }
	virtual ~Search() { }
	virtual Result<D> &search(D &, typename D::State &) = 0;

	virtual void reset(void) {
		res = Result<D>();
		
	}

	virtual void output(FILE *f) {
		lim.output(f);
		res.output(f);
	}

protected:
	bool limit(void) { return lim.reached(res); }
	Result<D> res;
	Limit lim;
};

#endif	// _SEARCH_HPP_
