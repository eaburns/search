#include "mdist.hpp"
#include "../search/main.hpp"
#include <cstdio>
#include <limits>

int main(int argc, const char *argv[]) {
	TilesMdist d(stdin);
	Result<TilesMdist> res = search<TilesMdist>(d, argc, argv);
	if (res.path.size() == 0)
		return 0;

	TilesMdist::State state = d.initialstate();
	TilesMdist::Cost cost = 0;

	if (res.ops.size() > (unsigned int) std::numeric_limits<int>::max())
		fatal("Too many actions");

	for (int i = res.ops.size() - 1; i >= 0; i--) {
		TilesMdist::State copy(state);
		TilesMdist::Edge e(d, copy, res.ops[i]);
		cost += e.cost;
		assert(e.state.eq(res.path[i]));
		state = e.state;
	}

	dfpair(stdout, "final sol cost", "%u", (unsigned int) cost);
	assert (d.isgoal(state));

	return 0;
}
