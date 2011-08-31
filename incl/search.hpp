 #include <cstdio>
#include <vector>

double walltime(void);
double cputime(void);

template <class D>
struct Result {
	double wallstrt, cpustrt;
	double wallend, cpuend;
	unsigned long expd, gend;
	long dups;
	typename D::Cost cost;
	std::vector<typename D::State> path;

	Result(bool dups = false) : expd(0), gend(0), dups(-1), cost(D::InfCost) {
		if (dups)
			dups = 0;
		start();
	}

	void start(void) {
		wallstrt = walltime();
		cpustrt = cputime();
	}

	void finish(void) {
		wallend = walltime();
		cpuend = cputime();
	}

	void output(FILE *f) {
		fprintf(f, "total CPU time: %g\n", cpuend - cpustrt);
		fprintf(f, "total wall time: %g\n", wallend - wallstrt);
		fprintf(f, "total nodes expanded: %lu\n", expd);
		fprintf(f, "total nodes generated: %lu\n", gend);
		if (dups >= 0)
			fprintf(f, "total nodes duplicated: %ld\n", dups);
		fprintf(f, "total solution cost: %g\n", (double) cost);
		fprintf(f, "total solution length: %lu\n", (unsigned long) path.size());
	}
};