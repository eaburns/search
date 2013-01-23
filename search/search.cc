#include "../search/search.hpp"
#include "../utils/utils.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sys/time.h>
#include <sys/resource.h>

static void settimer(int, unsigned long);
static void stoptimer(int);
static int timersig(int);

SearchStats::SearchStats() : 
	wallstart(0), cpustart(0), wallend(0), cpuend(0),
	expd(0), gend(0), reopnd(0), dups(0) { }

void SearchStats::start() {
	wallstart = walltime();
	cpustart = cputime();
}

void SearchStats::finish() {
	wallend = walltime();
	cpuend = cputime();
}

void SearchStats::output(FILE *f) {
	dfpair(f, "total raw cpu time", "%f", cpuend - cpustart);
	dfpair(f, "total wall time", "%f", wallend - wallstart);
	dfpair(f, "total nodes expanded", "%lu", expd);
	dfpair(f, "total nodes generated", "%lu", gend);
	dfpair(f, "total nodes duplicated", "%lu", dups);
	dfpair(f, "total nodes reopened", "%lu", reopnd);
}

Limit::Limit() :
	expd(0), gend(0), mem(0), cputime(0), walltime(0), timeup(0) { }

Limit::Limit(int argc, const char *argv[]) :
		expd(0), gend(0), mem(0), cputime(0), walltime(0), timeup(0) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-expd") == 0 && i < argc - 1)
			expd = strtoul(argv[++i], NULL, 10);
		else if (strcmp(argv[i], "-gend") == 0 && i < argc - 1)
			gend = strtoul(argv[++i], NULL, 10);
		else if (strcmp(argv[i], "-mem") == 0 && i < argc - 1)
			memlimit(argv[++i]);
		else if (strcmp(argv[i], "-cputime") == 0 && i < argc - 1)
			cputime = strtoul(argv[++i], NULL, 10);
		else if (strcmp(argv[i], "-walltime") == 0 && i < argc - 1)
			walltime = strtoul(argv[++i], NULL, 10);
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
		fatalx(errno, "failed to get the current memory limit");

	lim.rlim_cur =  lim.rlim_max > mem ? mem : lim.rlim_max;
	r = setrlimit(RLIMIT_AS, &lim);
	if (r != 0)
		fatalx(errno, "failed to set the memory limit");
}

// timed is the limit that is signalled when its time is up.
static Limit *timed;

void handletimeout(int) {
	timed->timeup = true;
}

void Limit::timelimit(const char *tstr) {
	if (timed)
		fatal("Only one time limit is supported");
	timed = this;
}

void Limit::start() {
	if (walltime == 0 && cputime == 0)
		return;
	if (!timed)
		timed = this;
	if (timed != this)
		fatal("Only one timed limit is supported");
	if (walltime > 0)
		settimer(ITIMER_REAL, walltime);
	if (cputime > 0)
		settimer(ITIMER_PROF, cputime);
}

void Limit::finish() {
	if (walltime == 0 && cputime == 0)
		return;
	if (walltime > 0)
		stoptimer(ITIMER_REAL);
	if (cputime > 0)
		stoptimer(ITIMER_PROF);
	timed = NULL;
}

// settimer sets the signal handler to handletimout
// and the timer to the given value.
static void settimer(int timer, unsigned long secs) {
	struct sigaction act = {};
	act.sa_handler = handletimeout;
	int sig = timersig(timer);
	if (sigaction(sig, &act, NULL) != 0)
		fatal("failed to set alarm (sig=%d) handler", sig);

	struct itimerval vl = { };
	vl.it_value.tv_sec = secs;
	if (setitimer(timer, &vl, NULL) != 0)
		fatalx(errno, "failed to set the timer (%d)", timer);
}

// stoptimer stops the timer and sets the
// signal handler back to the default.
static void stoptimer(int timer) {
	struct itimerval vl = { };
	if (setitimer(timer, &vl, NULL) != 0)
		fatalx(errno, "failed to stop the timer (%d)", timer);

	struct sigaction act = {};
	act.sa_handler = SIG_DFL;
	int sig = timersig(timer);
	if (sigaction(sig, &act, NULL) != 0)
		fatal("failed to set alarm (sig=%d) handler", sig);
}

// timersig gets the signal associated with the
// given timer.
static int timersig(int timer) {
	switch (timer) {
	case ITIMER_REAL:
		return SIGALRM;
	case ITIMER_PROF:
		return SIGPROF;
	case ITIMER_VIRTUAL:
		return SIGVTALRM;
	}
	fatal("unknown timer type: %d", timer);
	return -1;	// unreachable
}

void Limit::output(FILE *f) {
	if (expd > 0)
		dfpair(f, "expanded limit", "%lu", expd);
	if (gend > 0)
		dfpair(f, "generated limit", "%lu", gend);
	if (mem > 0)
		dfpair(f, "memory limit", "%lu", mem);
	if (cputime > 0)
		dfpair(f, "cpu time limit", "%lu", cputime);
	if (walltime > 0)
		dfpair(f, "wall time limit", "%lu", walltime);
}
