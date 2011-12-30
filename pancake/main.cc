#include "pancake.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	Pancake d(stdin);
	Result<Pancake> res = search<Pancake>(d, argc, argv);

	Pancake::State state = d.initialstate();
	Pancake::Cost cost = 0;
	for (int i = res.ops.size() - 1; i >= 0; i--) {
		Pancake::State copy(state);
		Pancake::Transition tr(d, copy, res.ops[i]);
		assert (tr.state.eq(res.path[i]));
		state = tr.state;
		cost += tr.cost;
	}
	assert (d.isgoal(state));
	assert (cost == res.cost);

	return 0;
}
