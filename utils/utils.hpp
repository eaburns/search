#include <cstdio>
#include <stdint.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <boost/cstdint.hpp>

void warn(const char *, ...);
void warnx(int, const char *, ...);
void fatal(const char*, ...);
void fatalx(int, const char*, ...);

double walltime(void);
double cputime(void);

// virtmem returns the maximum virtual memory usage
// of the current program in Kilobytes.
unsigned long virtmem(void);

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

struct RdbAttrs {
	bool push_back(const std::string &key, const std::string &val);	// false if duplicate

	void pop_front(void) {
		const std::string &k = keys.front();
		pairs.erase(pairs.find(k));
		keys.pop_front();
	}

	const std::string &front(void) { return keys.front(); }

	unsigned int size(void) const { return keys.size(); }

	bool rm(const std::string &key);

	const std::string &lookup(const std::string &key) { return pairs[key]; }

	bool mem(const std::string &key) const {
		return pairs.find(key) != pairs.end();
	}

private:
	std::map<std::string, std::string> pairs;
	std::deque<std::string> keys;
};

std::string rdbpathfor(const char *, RdbAttrs);

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

struct Ranker {
	// Rank the permutation 0..n-1
	Ranker(unsigned int n);

	// Permrank for a subset of the permutation
	// 0..(n-1) that contains sz elements.
	// Cannot be unranked!
	Ranker(unsigned int sz, unsigned int n);

	~Ranker(void);

	// The array must have ≤ sz elements.
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

unsigned int ilog2(boost::uint32_t);

struct Djforrest {
	Djforrest(unsigned int sz) {
		for (unsigned int i = 0; i < sz; i++)
			sets.push_back(Set(i));
	}

	unsigned int find(unsigned int s) {
		if (sets[s].parent == s)
			return s;
		sets[s].parent = find(sets[s].parent);
		return sets[s].parent;
	}

	void join(unsigned int a, unsigned int b) {
		unsigned int aroot = find(a);
		unsigned int broot = find(b);
		if (aroot == broot)
			return;
		if (sets[a].rank < sets[b].rank) {
			sets[a].parent = broot;
		} else if (sets[b].rank < sets[a].rank) {
			sets[b].parent = aroot;
		} else {
			sets[b].parent = aroot;
			sets[a].rank++;
		}
	}
	
	struct Set {
		Set(unsigned int n) : parent(n), rank(0) { }

		unsigned int parent, rank;
	};

private:
	std::vector<Set> sets; 
};

// Disjoint sets
struct Djset {
	Djset(void) : aux(NULL), rank(0) { parent = this; }

	void clear(void) {
		rank = 0;
		parent = this;
	}

	Djset *find(void) {
		if (parent == this)
			return parent;
		parent = parent->find();
		return parent;
	}

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

	void *aux;	// Inited to NULL. Never touched; can be used however.

private:
	Djset *parent;
	unsigned int rank;
};