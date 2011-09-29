#include "search.hpp"
#include "astar.hpp"
#include "idastar.hpp"
#include "../utils/utils.hpp"
#include <cstddef>
#include <cstdio>

template<class D> Search<D> *getsearch(int argc, char *argv[]);

template<class D> void search(D &d, int argc, char *argv[]) {
	Search<D> *srch = getsearch<D>(argc, argv);
	typename D::State s0 = d.initialstate();
	dfpair(stdout, "initial heuristic", "%d", d.h(s0));
	srch->search(d, s0);
	srch->output(stdout);
	dfprocstatus(stdout);

	delete srch;
}

template<class D> Search<D> *getsearch(int argc, char *argv[]) {
	if (strcmp(argv[1], "idastar") == 0)
		return new Idastar<D>(argc, argv);
	else if (strcmp(argv[1], "astar") == 0)
		return new Astar<D>(argc, argv);

	fatal("Unknown algorithm: %s\n", argv[1]);
	return NULL;	// Unreachable
}