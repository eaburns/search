#include "drobot.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	DockRobot d(stdin);
	Result<DockRobot> res = search<DockRobot>(d, argc, argv);

	DockRobot::State state = d.initialstate();
	DockRobot::Cost cost(0);
	for (int i = res.ops.size() - 1; i >= 0; i--) {
		DockRobot::State copy(state);
		DockRobot::Edge e(d, copy, res.ops[i]);
		assert (e.state == res.path[i]);
		state = e.state;
		cost += e.cost;
	}

	assert (res.path.size() == 0 || d.isgoal(state));
	if (res.path.size() == 0)
		cost = -1;
	dfpair(stdout, "final sol cost", "%u", (unsigned int) cost);

	return 0;
}