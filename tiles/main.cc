#include "tiles.hpp"
#include "../search/idastar.hpp"
#include <cstdio>

int main(int argc, char *argv[]) {
	TilesMdist t(stdin);
	TilesMdist::State s0 = t.initstate();

	Result<TilesMdist> res = Idastar::search<TilesMdist>(&t, &s0);

	printf("%u cost\n", res.cost);

	return 0;
}