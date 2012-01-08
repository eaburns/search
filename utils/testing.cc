#include "../utils/utils.hpp"
#include <cstdarg>
#include <cstdio>
#include <string>
#include <limits>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <sys/types.h>
#include <regex.h>

enum { MaxN = 1000000000 };
static const double Mintime = 1.0;	// seconds

static std::string msg;

static bool runtest(const Test &);
static void runbench(const Benchmark &);

bool runtests(const Test tests[], int num, const char *regexp) {
	regex_t re;
	int err = regcomp(&re, regexp, 0);
	if (err)
		fatalx(err, "Failed to compile regexp [%s]", regexp);

	unsigned int nfailed = 0, ntotal = 0;
	for (int i = 0; i < num; i++) {
		if (regexec(&re, tests[i].name, 0, NULL, 0) == REG_NOMATCH)
			continue;
		if (!runtest(tests[i]))
			nfailed++;
		ntotal++;
	}

	printf("%u tests failed\n", nfailed);
	printf("%u tests passed\n", ntotal - nfailed);
	printf("%u tests total\n", ntotal);

	return nfailed == 0;
}

void runbenches(const Benchmark benchs[], int num, const char *regexp) {
	regex_t re;
	int err = regcomp(&re, regexp, 0);
	if (err)
		fatalx(err, "Failed to compile regexp [%s]", regexp);

	for (int i = 0; i < num; i++) {
		if (regexec(&re, benchs[i].name, 0, NULL, 0) == REG_NOMATCH)
			continue;
		runbench(benchs[i]);
	}
}

void testpr(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	int sz = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	char buf[sz+1];
	va_start(ap, fmt);
	vsnprintf(buf, sz+1, fmt, ap);
	va_end(ap);

	msg.append(buf);
}

static bool runtest(const Test &t) {
	msg.clear();
	bool ok = t.run();

	printf("Running %-40s	", t.name);
	fflush(stdout);

	if (ok)
		printf("ok\n");
	else
		printf("FAILED\n");

	if (msg.size() > 0)
		puts(msg.c_str());

	return ok;
}

static void runbench(const Benchmark &b) {
	msg.clear();

	printf("Running %-40s ", b.name);

	double ttime = 1.0;
	unsigned long n = -1;
	for (n = 10; n < MaxN; n *= 10) {
		double strt = walltime(), end = 0.0;
		b.run(n, &strt, &end);

		if (fabs(end) < std::numeric_limits<double>::epsilon())
			end = walltime();

		ttime = end - strt;
		if (ttime > Mintime)
			break;
	}

	printf("%15s op/s (%.1gs, n=%g)\n",
		commas("%lu", (unsigned long) (n / ttime)).c_str(),
		ttime, (double) n);

	if (msg.size() > 0)
		puts(msg.c_str());
}

unsigned int *randuints(unsigned long n) {
	static unsigned int *ints;
	static unsigned long nints;

	if (nints < n) {
		if (ints)
			delete[] ints;
		ints = new unsigned int[n];
		nints = n;
		for (unsigned long i = 0;  i < n; i++)
			ints[i] = randgen.bits();
	}

	return ints;
}

unsigned int *scratchuints(unsigned long n) {
	static unsigned int *ints;
	static unsigned long nints;

	if (nints < n) {
		if (ints)
			delete[] ints;
		ints = new unsigned int[n];
		nints = n;
	}

	return ints;
}

double *randdoubles(unsigned long n) {
	static double *ds;
	static unsigned long nds;

	if (nds < n) {
		if (ds)
			delete[] ds;
		ds = new double[n];
		nds = n;
		for (unsigned long i = 0;  i < n; i++)
			ds[i] = randgen.real();
	}

	return ds;
}

double *scratchdoubles(unsigned long n) {
	static double *ds;
	static unsigned long nds;

	if (nds < n) {
		if (ds)
			delete[] ds;
		ds = new double[n];
		nds = n;
	}

	return ds;
}

std::string commas(const char *fmt, ...) {
	char str[128];
	memset(str, 'a', sizeof(str));

	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	if (n < 0)
		fatal("commas: Failed to write format to the buffer");
	if ((unsigned int) n >= sizeof(str))
		fatal("commas: Buffer size is too small");

	char *end = str + n;
	assert (*end == '\0');
	assert (end[-1] != '\0');

	char *mid = strrchr(str, '.');
	if (!mid)
		mid = end;
	assert (*mid == '\0' || *mid == '.');

	char out[n + (n + 1)/3 + 1];
	memset(out, 'b', sizeof(out));
	char *cur = out + sizeof(out) - 1;
	assert (*cur == 'b');

	for (char *p = end; p >= mid; p--, cur--)
		*cur = *p;

	unsigned int tillcomma = 3;
	for (char *p = mid-1; p >= str; p--, cur--) {
		*cur = *p;
		tillcomma--;
		if (tillcomma == 0 && p > str) {
			cur--;
			*cur = ',';
			tillcomma = 3;
		}
	}

	return cur + 1;
}
