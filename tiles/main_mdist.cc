#include "mdist.hpp"
#include "../search/main.hpp"
#include <cstdio>
#include <limits>

int main(int argc, const char *argv[]) {
	TilesMdist d(stdin);
	Result<TilesMdist> res = search<TilesMdist>(d, argc, argv);
	if (res.cost < 0)
		return 0;

	TilesMdist::State state = d.initialstate();
	TilesMdist::Cost cost = 0;

	if (res.ops.size() > (unsigned int) std::numeric_limits<int>::max())
		fatal("Too many actions");

	for (int i = res.ops.size() - 1; i >= 0; i--) {
		TilesMdist::State copy(state);
		TilesMdist::Transition tr(d, copy, res.ops[i]);
		cost += tr.cost;
		assert(tr.state.eq(res.path[i]));
		state = tr.state;
	}

	assert (res.cost == cost);
	assert (d.isgoal(state));

	return 0;
}
