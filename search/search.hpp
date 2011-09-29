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
	virtual Result<D> &search(D &, typename D::State &) = 0;
	Search(int argc, char *argv[]) : lim(argc, argv) { }

	void output(FILE *f) {
		lim.output(f);
		res.output(f);
	}

protected:
	bool limit(void) { return lim.reached(res); }
	Result<D> res;
	Limit lim;
};


#endif	// _SEARCH_HPP_
