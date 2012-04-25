#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <cstdio>
#include <string>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

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

// readline reads a line from the given file stream and returns it
// without the trailing newline. Returns an empty option on
// end of file and calls fatal on error.  If echo is non-NULL then
// each line is echoed to the file after it is read.
boost::optional<std::string> readline(FILE*, FILE *echo = NULL);

// readdir returns all of the entries of the given directory
// excluding '.' and '..'.
// If concat is true then the directory is concatinated with the
// entry name, otherwise only the entry is returned.
std::vector<std::string> readdir(std::string, bool concat = true);

// fileexists returns true if the given file exists.
bool fileexists(const std::string&);

// isdir returns true if the given path is a directory.
// If the path does not exist then false is returned,
// other errors are fatal.
bool isdir(const std::string&);

// ensuredir ensures that the directory for the given
// path exists.
void ensuredir(const std::string&);

// recursively removes the given file(s) and return true
// if they were all removed and false otherwise.  If a file
// cannot be removed then a warning is printed.
bool rmrecur(const std::string&);

// basename returns the basename of the given path.
// The basename is the entry to the right of the right-most
// path seperator.
std::string basename(const std::string&);

// dirname returns the directory containing the given
// path, i.e., everything to the left of the right-most
// path seperator.
std::string dirname(const std::string&);

// pathcat returns the concatination of two path entries.
std::string pathcat(const std::string&, const std::string&);

// pathcatn returns the concatination of a bunch of path
// entries specified by first argument.  The remaining
// arguments are assumed to be const char *'s.
std::string pathcatn(unsigned int, ...);

// hasprefix returns true if the first string has the second
// string as a prefix.
bool hasprefix(const char*, const char*);

// gettoken returns the first whitespace delimited or quoted
// token from the string.  This routine  modifies the given
// string, returns NULL if end of line is encountered before a
// token.  If lineno > 0 then it is printed in the case that a
// quote match fails.
char *gettoken(char*, unsigned int lineno = 0);

// tokens returns a vector of the space delimited tokens in
// the given string.  Quotes are interpreted as strings, and so
// spaces within quotes do not delimit tokens.
std::vector<std::string> tokens(const std::string&);

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
void dfrowhdr(FILE *, const char *name, unsigned int ncols, ...);

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

typedef void(*Dfhandler)(std::vector<std::string>&, void*);

// dfread reads #pair, #altcols and #altrow lines from the
// given datafile and calls the handler for each.  The vector
// passed to the handler fast the type of line as the 0th
// element (#pair, #altcols or #altrow) and has the remaining
// tokes of the line as its remaining aruments.  If echo is
// non-NULL then the lines are echoed to this file immediately
// after they are read.
void dfread(FILE*, Dfhandler, void *priv = NULL, FILE *echo = NULL);

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
bool runtests(const Test [], unsigned int, const char *regexp);

// runbenches runs all of the benchmarks that match the given
// regular expression.  Timing information is printed for each
// benchmark.
void runbenches(const Benchmark[], unsigned int, const char *regexp);

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

// ipow returns the value of the first argument raised
// to the power of the second argument.
unsigned long ipow(unsigned int, unsigned int);

// fallfact returns `x to the n falling.'
unsigned long fallfact(unsigned int x, unsigned int n);

#endif	// _UTILS_HPP_