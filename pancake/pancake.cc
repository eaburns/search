// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "pancake.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cerrno>

Pancake::Pancake(FILE *in) {
	unsigned int ncakes;
	if (fscanf(in, "%u", &ncakes) != 1)
		fatalx(errno, "Failed to read the number of pancakes");

	if (ncakes != Ncakes)
		fatal("Number of pancakes instance/compiler option mismatch");

	for (unsigned int i = 0; i < Ncakes; i++) {
		if (fscanf(in, " %d", init + i) != 1)
			fatalx(errno, "Failed to read pancake number %d", i);
	}
}

Pancake::State Pancake::initialstate() {
	State s;

	for (unsigned int i = 0; i < Ncakes; i++)
		s.cakes[i] = init[i];
	s.h = ngaps(s.cakes);

	return s;
}

Pancake::Cost Pancake::pathcost(const std::vector<State> &path, const std::vector<Oper> &ops) {
	State state = initialstate();
	Cost cost(0);
	for (int i = ops.size() - 1; i >= 0; i--) {
		State copy(state);
		Edge e(*this, copy, ops[i]);
		assert (e.state.eq(this, path[i]));
		state = e.state;
		cost += e.cost;
	}
	assert (isgoal(state));
	return cost;
}