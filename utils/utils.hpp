#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <cstdio>
#include <stdint.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <boost/cstdint.hpp>

// warn prints the formatted warning message.
void warn(const char *, ...);

// warnx prints the formatted warning message along with the
// string representation of the errno value.
void warnx(int, const char *, ...);

// fatal prints a formated error message and exits with failure
// status.
void fatal(const char*, ...);

// fatalx prints a formatted error message along with the error
// string for the errno and exits with failure status.
void fatalx(int, const char*, ...);

// walltime returns the current wall-cloce time in seconds
double walltime(void);

// cputime returns the current CPU time in seconds.
double cputime(void);

// virtmem returns the maximum virtual memory usage
// of the current program in Kilobytes.
unsigned long virtmem(void);

// dfpair writes a datafile-formatted key/value pair to the
// given output file.
void dfpair(FILE *, const char *key, const char *fmt, ...);

// dfrowhdr writes 'altcol' header information to the given file.
// ncols specifies the number of columns and the variadic
// arguments be ncols strings representing the column headers.
void dfrowhdr(FILE *, const char *name, int ncols, ...);

// dfrow writes an 'altcols' row to the given file.
// colfmt is a string of characters: g, f, d, and u with the
// following meaning:
// g is a %g formatted double,
// f is a %f formatted double
// d is a %ld formatted long
// u is a %lu formatted unsigned long
void dfrow(FILE *, const char *name, const char *colfmt, ...);

// dfheader writes the datafile format header information to
// the file.
void dfheader(FILE *);

// dffooter writes the datafile format footer information to
// the file.
void dffooter(FILE *);

// dfprocstatus writes status from the /proc filesystem to the file
// if the proc/ filesystem is readable.
void dfprocstatus(FILE*);

typedef void(*Pairhandler)(const char*, const char*, void*);

// dfreadpairs reads pairs from the given datafile and calls
// the handler for each pair.  If echo is true then the lines that
// are read are echoed back out standard output.
void dfreadpairs(FILE*, Pairhandler, void *priv = NULL, bool echo = false);

// The RdbAttrs struct holds attributes for the RDB database.
// It behaves as a FIFO queue of attributes.
struct RdbAttrs {

	// push_back adds a new key=value pair to the attribute
	// vector.  It returs false if the key is already added as one
	// of the attributes.
	bool push_back(const std::string &key, const std::string &val);

	// pop_front removes the first attribute that was added to the
	// list.
	void pop_front(void) {
		const std::string &k = keys.front();
		pairs.erase(pairs.find(k));
		keys.pop_front();
	}

	// front returns the key for the next attribute pair.
	const std::string &front(void) { return keys.front(); }

	// size returns the number of key=value pairs.
	unsigned int size(void) const { return keys.size(); }

	// rm removes the pair for the given key.
	bool rm(const std::string &key);

	// lookup returns the value associated with the given
	// key.
	const std::string &lookup(const std::string &key) { return pairs[key]; }

	// mem returns true if there is a value bound to the given
	// key.
	bool mem(const std::string &key) const {
		return pairs.find(key) != pairs.end();
	}

private:
	std::map<std::string, std::string> pairs;
	std::deque<std::string> keys;
};

// rdbpathfor returns the filesystem path for the given attribute
// set rooted at the given root directory.
std::string rdbpathfor(const char *, RdbAttrs);

// rdbwithattrs returns a vector of all file paths that have the given
// attributes under the given root directory.
std::vector<std::string> rdbwithattrs(const char*, RdbAttrs);

// A Test holds information on a unit test that may be run
// via the testing framework.
struct Test {
	const char *name;
	bool (*run)(void);

	Test(const char *n, bool (*r)(void)) : name(n), run(r) {}
};

// A benchmark holds information on a benchmark that
// may be run via the testing framework.
struct Benchmark {
	const char *name;
	void (*run)(unsigned long n, double *strt, double *end);

	Benchmark(const char *n, void (*r)(unsigned long, double *, double *)) :
		name(n), run(r) {}
};

// runtests runs all of the tests in the list that match the given
// regular expression.  Information on each test and the number
// of passed and failed tests is printed to standard output.
bool runtests(const Test [], int, const char *regexp);

// runbenches runs all of the benchmarks that match the given
// regular expression.  Timing information is printed for each
// benchmark.
void runbenches(const Benchmark[], int, const char *regexp);

// testpr can be used to display formatted output from within
// a testing function.  The output is surpressed until the test is
// completed.  Upon completion the formatted output is printed
// to standard output.
void testpr(const char *fmt, ...);

// commas returns the string format of a number with
// commas inserted every 3-tens places.  The fmt argument
// is the printf-style format specifier for the number that
// follows as a variadic argument.
// 
// BUG: commas does not format the output correctly
// with the %g or %e forms of floats or doubles.
std::string commas(const char *fmt, ...);

// hashbytes computes a hash on an arbitrary array of bytes.
// The hash function is by Bob Jenkins (2006) and the resulting
// hash value is 32-bits.
extern "C" unsigned long hashbytes(unsigned char[], unsigned int);

// Rand is pseudo-random number generator (RNG1 from
// Numerical Recipes).
class Rand {
public:
	// Rand creates a new Rand with the given seed.
	Rand(unsigned long);

	// bits returns the next 64 pseudo-random bits.
	unsigned long bits(void);

	// integer returns a pseudo-random integer between min
	// and max, inclusive.
	long integer(long min, long max);

	// real returns the a pseudo-random double between
	// 0 and 1 (not sure if it's inclusive).
	double real(void);

	unsigned long seed(void) const { return theseed; }
private:
	unsigned long theseed;
	uint64_t v;
};

// randgen is a pre-seeded pseudo-random number generator that
// is seeded with the current time upon program initialization.
extern Rand randgen;

// runlenenc performs a run-length encoding of the given string
// of data.  The result is returned via the dst argument.
std::string runlenenc(const std::string &data);

// runlendec decodes a run-length encoded string.
std::string runlendec(const std::string &data);

// ascii85enc encodes a string using ASCII-85 encoding.
std::string ascii85enc(const std::string &data);

// base64enc encodes a string using MIME's base64 encoding.
std::string base64enc(const std::string &data);

// base64dec decodes a MIME base64 encoded string.
std::string base64dec(const std::string &data);

// The Ranker type generates permutation ranks.
struct Ranker {
	// Ranker creates a new Ranker for the permutation
	// 0..n-1
	Ranker(unsigned int n);

	// Ranker generates a Ranker for a subset of the
	// permutation 0..(n-1) that contains sz elements.
	// Ranks made from a Ranker created with this
	// constructor cannot be unranked!  This is one-way.
	Ranker(unsigned int sz, unsigned int n);

	~Ranker(void);

	// rank returns the permutation rank for the given
	// permutation. The array must have ≤ sz elements.
	unsigned long rank(const unsigned int []);

private:
	unsigned int sz;	// permutation size
	unsigned int bits;	// ilog2(sz)

	// Number of perm elements in ranked arrays.
	// This may be ≤sz.  If it is equal to sz then the
	// ranker ranks full permutations of sz.  If it is
	// less than sz then the ranker ranks vectors of
	// n elements of sz.  They cannot be unranked.
	unsigned int n;

	unsigned int *tree;
	unsigned int treesz;
};

// ilog2 returns the interger logorithm base 2.
unsigned int ilog2(boost::uint32_t);

static inline unsigned long ipow(unsigned int b, unsigned int e) {
	unsigned long r = 1;
	for (unsigned int i = 0; i < e; i++)
		r *= b;
	return r;
}

// The Djset type is a disjoint forrest implementation of disjoint
// sets.
struct Djset {

	// Djset creates a new set.
	Djset(void) : aux(NULL), rank(0) { parent = this; }

	// clear resets the set as though it has just been freshly
	// created.
	void clear(void) {
		rank = 0;
		parent = this;
	}

	// find returns the canonical representation of this set.
	Djset *find(void) {
		if (parent == this)
			return parent;
		parent = parent->find();
		return parent;
	}

	// join unions two sets so that performing a find on either
	// will return the same canonical set.
	void join(Djset &o) {
		Djset *root = find();
		Djset *oroot = o.find();
		if (root == oroot)
			return;
		if (root->rank < oroot->rank) {
			root->parent = oroot;
		} else {
			oroot->parent = root;
			if (root->rank == oroot->rank)
				rank++;
		}
	}

	// aux is initialized to NULL and is never again touched by
	// this code.  This may be used however you please.
	void *aux;

private:
	Djset *parent;
	unsigned int rank;
};

#endif	// _UTILS_HPP_