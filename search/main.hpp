#include "search.hpp"
#include "idastar.hpp"
#include "astar.hpp"
#include "wastar.hpp"
#include "greedy.hpp"
#include "bugsy.hpp"
#include "arastar.hpp"
#include "rtastar.hpp"
#include <cstddef>
#include <cstdio>

// Functions for conveniently defining a new main
// function for a domain.

void dfheader(FILE*);
void dffooter(FILE*);
void dfpair(FILE *, const char *, const char *, ...);
void dfprocstatus(FILE*);
void fatal(const char*, ...);

template<class D> SearchAlgorithm<D> *getsearch(int argc, const char *argv[]);
bool headerfooter(int, const char*[]);

template<class D> Result<D> search(D &d, int argc, const char *argv[]) {
	SearchAlgorithm<D> *srch = getsearch<D>(argc, argv);
	if (!srch && argc > 1)
		fatal("Unknow search algorithm: %s", argv[1]);
	if (!srch)
		fatal("Must specify a search algorithm");

	typename D::State s0 = d.initialstate();
	if (headerfooter(argc, argv))
		dfheader(stdout);
	dfpair(stdout, "initial heuristic", "%g", (double) d.h(s0));
	dfpair(stdout, "initial distance", "%g", (double) d.d(s0));
	dfpair(stdout, "algorithm", argv[1]);

	try {
		srch->search(d, s0);
	} catch (std::bad_alloc&) {
		srch->finish();
	}
	srch->output(stdout);
	if (headerfooter(argc, argv))
		dffooter(stdout);

	Result<D> res = srch->res;
	delete srch;

	return res;
}

template<class D> SearchAlgorithm<D> *getsearch(int argc, const char *argv[]) {
	if (argc < 2)
		fatal("No algorithm specified");

	if (strcmp(argv[1], "idastar") == 0)
		return new Idastar<D>(argc, argv);
	else if (strcmp(argv[1], "astar") == 0)
		return new Astar<D>(argc, argv);
	else if (strcmp(argv[1], "wastar") == 0)
		return new Wastar<D>(argc, argv);
	else if (strcmp(argv[1], "greedy") == 0)
		return new Greedy<D>(argc, argv);
	else if (strcmp(argv[1], "speedy") == 0)
		return new Greedy<D, true>(argc, argv);
	else if (strcmp(argv[1], "bugsy") == 0)
		return new Bugsy<D>(argc, argv);
	else if (strcmp(argv[1], "arastar") == 0)
		return new Arastar<D>(argc, argv);
	else if (strcmp(argv[1], "arastarmon") == 0)
		return new ArastarMon<D>(argc, argv);
	else if (strcmp(argv[1], "rtastar") == 0)
		return new Rtastar<D>(argc, argv);

	fatal("Unknown algorithm: %s", argv[1]);
	return NULL;	// Unreachable
}
