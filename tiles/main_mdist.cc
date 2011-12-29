#include "mdist.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	TilesMdist d(stdin);
	Result<TilesMdist> res = search<TilesMdist>(d, argc, argv);

	TilesMdist::State state = d.initialstate();
	TilesMdist::Cost cost = 0;
	for (int i = res.ops.size() - 1; i >= 0; i--) {
		TilesMdist::Cost c;
		TilesMdist::State buf, &kid = d.apply(buf, state, c, res.ops[i]);
		cost += c;
		assert(kid.eq(res.path[i]));
		state = kid;
	}
	assert (res.cost == cost);

	return 0;
}