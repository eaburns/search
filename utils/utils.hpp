#include <cstdio>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>

void warn(const char *, ...);
void warnx(int, const char *, ...);
void fatal(const char*, ...);
void fatalx(int, const char*, ...);

double walltime(void);
double cputime(void);

void dfpair(FILE *, const char *key, const char *fmt, ...);
void dfrowhdr(FILE *, const char *name, int ncols, ...);
// colfmt is a string of characters: g, f, d, and u with the
// following meaning:
// g is a %g formatted double,
// f is a %f formatted double
// d is a %ld formatted long
// u is a %lu formatted unsigned long
void dfrow(FILE *, const char *name, const char *colfmt, ...);
void dfheader(FILE *);
void dffooter(FILE *);
void dfprocstatus(FILE*);

// Read pairs from the data file and pass each to
// // the given function.
typedef void(*Pairhandler)(const char*, const char*, void*);
void dfreadpairs(FILE*, Pairhandler, void *priv = NULL, bool echo = false);

typedef std::map<std::string, std::string> RdbAttrs ;

struct Test {
	const char *name;
	bool (*run)(void);

	Test(const char *n, bool (*r)(void)) : name(n), run(r) {}
};

struct Benchmark {
	const char *name;
	void (*run)(unsigned long n, double *strt, double *end);

	Benchmark(const char *n, void (*r)(unsigned long, double *, double *)) :
		name(n), run(r) {}
};

bool runtests(const Test [], int, const char *regexp);
void runbenches(const Benchmark[], int, const char *regexp);
void testpr(const char *fmt, ...);

extern "C" unsigned long hashbytes(unsigned char[], unsigned int);

class Rand {
public:
	Rand(unsigned long);
	unsigned long bits(void);
	// (not sure if it's inclusive or exclusive)
	long integer(long min, long max);
	// between 0 and 1 (not sure if it's inclusive or exclusive)
	double real(void);

	unsigned long seed(void) const { return theseed; }
private:
	unsigned long theseed;
	uint64_t v;
};

// A global, pre-seeded random number generator.
extern Rand randgen;

void runlenenc(std::string &dst, const std::string &data);
void runlendec(std::string &dst, const std::string &data);
void ascii85enc(std::string &dst, const std::string &data);
