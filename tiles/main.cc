#include "mdist.hpp"
#include "../search/idastar.hpp"
#include "../search/astar.hpp"
#include "../incl/utils.hpp"
#include <cstdio>

static void search(TilesMdist&, Search<TilesMdist>&, TilesMdist::State&);
static Search<TilesMdist> *getsearch(int, char *[]);

int main(int argc, char *argv[]) {
	TilesMdist d(stdin);
	Search<TilesMdist> *srch = getsearch(argc, argv);
	TilesMdist::State s0 = d.initstate();
	search(d, *srch, s0);
	delete srch;
	return 0;
}

static void search(TilesMdist &d, Search<TilesMdist> &srch, TilesMdist::State &s0) {
	dfheader(stdout);
	dfpair(stdout, "initial heuristic", "%d", d.h(s0));
	Result<TilesMdist> res = srch.search(d, s0);
	res.output(stdout);
	dffooter(stdout);
}

static Search<TilesMdist> *getsearch(int argc, char *argv[]) {
	Search<TilesMdist> *srch = NULL;

	if (argc > 1 && strcmp(argv[1], "idastar") == 0)
		srch = new Idastar<TilesMdist, true>();
	else
		srch = new Astar<TilesMdist>();

	return srch;
}
