#include "visnav.hpp"
#include "../search/main.hpp"
#include <cstdio>
#include <cerrno>

int main(int argc, const char *argv[]) {
	VisGraph g(stdin);
	double x0, y0, x1, y1;
	if (fscanf(stdin, " %lg %lg %lg %lg", &x0, &y0, &x1, &y1) != 4)
		fatal("Failed to read start and finish locations");

	dfpair(stdout, "x0", "%g", x0);
	dfpair(stdout, "y0", "%g", y0);
	dfpair(stdout, "x1", "%g", x1);
	dfpair(stdout, "y1", "%g", y1);
	VisNav d(g, x0, y0, x1, y1);
	Result<VisNav> res = search<VisNav>(d, argc, argv);

	VisNav::State state = d.initialstate();
	VisNav::Cost cost(0);
	for (int i = res.ops.size() - 1; i >= 0; i--) {
		VisNav::State copy(state);
		VisNav::Edge e(d, copy, res.ops[i]);
		assert (e.state == res.path[i]);
		state = e.state;
		cost += e.cost;
	}
	assert (d.isgoal(state));
	dfpair(stdout, "final sol cost", "%g", (double) cost);
	d.save("path.eps", res.path);

	return 0;
}