#include "search.hpp"
#include "astar.hpp"
#include "wastar.hpp"
#include "greedy.hpp"
#include "speedy.hpp"
#include "idastar.hpp"
#include <cstddef>
#include <cstdio>

// Functions for conveniently defining a new main
// function for a domain.

void dfpair(FILE *, const char *, const char *, ...);
void dfprocstatus(FILE*);
void fatal(const char*, ...);

template<class D> Search<D> *getsearch(int argc, char *argv[]);

template<class D> Result<D> search(D &d, int argc, char *argv[]) {
	Search<D> *srch = getsearch<D>(argc, argv);

	typename D::State s0 = d.initialstate();
	dfpair(stdout, "initial heuristic", "%g", (double) d.h(s0));
	dfpair(stdout, "algorithm", argv[1]);

	Result<D> res = srch->search(d, s0);
	srch->output(stdout);
	dfprocstatus(stdout);

	delete srch;

	return res;
}

template<class D> Search<D> *getsearch(int argc, char *argv[]) {
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
		return new Speedy<D>(argc, argv);

	fatal("Unknown algorithm: %s", argv[1]);
	return NULL;	// Unreachable
}