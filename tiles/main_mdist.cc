#include "mdist.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	TilesMdist d(stdin);
	Result<TilesMdist> res = search<TilesMdist>(d, argc, argv);
	if (res.cost < 0)
		return 0;

	TilesMdist::State state = d.initialstate();
	TilesMdist::Cost cost = 0;
	for (int i = res.ops.size() - 1; i >= 0; i--) {
		TilesMdist::State copy(state);
		TilesMdist::Transition tr(d, copy, res.ops[i]);
		cost += tr.cost;
		assert(tr.state.eq(res.path[i]));
		state = tr.state;
	}
	assert (d.isgoal(state));
	assert (res.cost == cost);

	return 0;
}
