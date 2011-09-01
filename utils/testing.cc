#include "../incl/utils.hpp"
#include <cstdarg>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <regex.h>

enum { MaxN = 10000000 };
static const double Mintime = 1.0;	// seconds

static bool runtest(const Test &);
static void runbench(const Benchmark &);

static std::string msg;

bool runtests(const Test tests[], int num, const char *regexp) {
	regex_t re;
	int err = regcomp(&re, regexp, 0);
	if (err)
		fatalx(err, "Failed to compile regexp [%s]\n", regexp);

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
		fatalx(err, "Failed to compile regexp [%s]\n", regexp);

	for (int i = 0; i < num; i++) {
		if (regexec(&re, benchs[i].name, 0, NULL, 0) == REG_NOMATCH)
			continue;
		runbench(benchs[i]);
	}
}

enum { Bufsz = 256 };

void testpr(const char *fmt, ...) {
	char buf[Bufsz];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, Bufsz, fmt, ap);
	va_end(ap);

	msg += buf;
}

static bool runtest(const Test &t) {
	msg.clear();
	bool ok = t.run();

	printf("Running %s...", t.name);
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

	printf("Running %s...\t", b.name);

	double ttime = 1.0;
	unsigned long n = -1;
	for (n = 10; n < MaxN; n *= 10) {
		double strt = walltime(), end = 0.0;
		b.run(n, &strt, &end);

		if (end == 0)
			end = walltime();

		ttime = end - strt;
		if (ttime > Mintime)
			break;
	}

	printf("%lu op/s (%g sec)\n", (unsigned long) (n / ttime),
		ttime);

	if (msg.size() > 0)
		puts(msg.c_str());
}
