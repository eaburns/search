#include "pancake.hpp"
#include "../search/idastar.hpp"
#include "../search/astar.hpp"
#include "../incl/utils.hpp"
#include <cstdio>

typedef Pancake Domain;

static void search(Domain&, Search<Domain>&, Domain::State&);
static Search<Domain> *getsearch(int, char *[]);

int main(int argc, char *argv[]) {
	Domain d(stdin);
	Search<Domain> *srch = getsearch(argc, argv);
	Domain::State s0 = d.initstate();
	search(d, *srch, s0);
	delete srch;
	return 0;
}

static void search(Domain &d, Search<Domain> &srch, Domain::State &s0) {
	dfpair(stdout, "initial heuristic", "%d", d.h(s0));
	Result<Domain> res = srch.search(d, s0);
	srch.output(stdout);
	dfprocstatus(stdout);
}

static Search<Domain> *getsearch(int argc, char *argv[]) {
	Search<Domain> *srch = NULL;

	if (argc > 1 && strcmp(argv[1], "idastar") == 0)
		srch = new Idastar<Domain, true>(argc, argv);
	else
		srch = new Astar<Domain>(argc, argv);

	return srch;
}
