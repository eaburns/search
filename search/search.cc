#include "../search/search.hpp"
#include "../utils/utils.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sys/time.h>
#include <sys/resource.h>


SearchStats::SearchStats(void) : 
	wallstrt(0), cpustrt(0), wallend(0), cpuend(0),
	expd(0), gend(0), reopnd(0), dups(0) { }

void SearchStats::start(void) {
	wallstrt = walltime();
	cpustrt = cputime();
}

void SearchStats::finish(void) {
	wallend = walltime();
	cpuend = cputime();
}

void SearchStats::output(FILE *f) {
	dfpair(f, "total raw cpu time", "%g", cpuend - cpustrt);
	dfpair(f, "total wall time", "%g", wallend - wallstrt);
	dfpair(f, "total nodes expanded", "%lu", expd);
	dfpair(f, "total nodes generated", "%lu", gend);
	dfpair(f, "total nodes duplicated", "%lu", dups);
	dfpair(f, "total nodes reopened", "%lu", reopnd);
}

Limit::Limit(void) : expd(0), gend(0), mem(0) { }

Limit::Limit(int argc, const char *argv[]) : expd(0), gend(0) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-expd") == 0 && i < argc - 1)
			expd = strtoul(argv[++i], NULL, 10);
		else if (strcmp(argv[i], "-gend") == 0 && i < argc - 1)
			gend = strtoul(argv[++i], NULL, 10);
		else if (strcmp(argv[i], "-mem") == 0 && i < argc - 1)
			memlimit(argv[++i]);
	}
}

void Limit::memlimit(const char *mstr) {
	char *endptr;
	double m = strtod(mstr, &endptr);

	if (*endptr == 'G')
		mem = m * 1e9;
	else if (*endptr == 'M')
		mem = m *1e6;
	else if (*endptr == 'K')
		mem = m *1e3;
 			else if (*endptr != '\0')
		fatal("failed to parse memory value %s\n", mstr);
	else
		mem = m;

	struct rlimit lim;
	int r = getrlimit(RLIMIT_AS, &lim);
	if (r != 0)
		fatalx(errno, "failed to get resource limit");

	lim.rlim_max = lim.rlim_max > mem ? mem : lim.rlim_max;
	lim.rlim_cur = lim.rlim_max;
	r = setrlimit(RLIMIT_AS, &lim);
	if (r != 0)
		fatalx(errno, "failed to set resource limit");
}

void Limit::output(FILE *f) {
	if (expd > 0)
		dfpair(f, "expanded limit", "%lu", expd);
	if (gend > 0)
		dfpair(f, "generated limit", "%lu", gend);
	if (mem > 0)
		dfpair(f, "memory limit", "%lu", mem);
}