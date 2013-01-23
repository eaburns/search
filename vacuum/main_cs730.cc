#include "vacuum.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {

	auto *srch = getsearch<Vacuum>(argc, argv);
	if (!srch && argc > 1)
		fatal("Unknow search algorithm: %s", argv[1]);
	if (!srch)
		fatal("Must specify a search algorithm");

	Vacuum d(stdin);

	auto s0 = d.initialstate();
	try {
		srch->search(d, s0);
	} catch (std::bad_alloc&) {
		dfpair(stdout, "out of memory", "%s", "true");
		srch->res.path.clear();
		srch->res.ops.clear();
		srch->finish();
	}

	d.printpath(stdout, srch->res.ops);
	fprintf(stdout, "%lu nodes generated\n", srch->res.gend);
	fprintf(stdout, "%lu nodes expanded\n", srch->res.expd);

	return 0;
}
