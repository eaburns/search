#include "pancake.hpp"
#include "../incl/utils.hpp"
#include <cstdio>
#include <cerrno>

Pancake::Pancake(FILE *in) {
	unsigned int ncakes;
	if (fscanf(in, "%u", &ncakes) != 1)
		fatalx(errno, "Failed to read the number of pancakes\n");

	if (ncakes != Ncakes)
		fatal("Number of pancakes instance/compiler option mismatch\n");

	for (unsigned int i = 0; i < Ncakes; i++) {
		if (fscanf(in, " %d", init + i) != 1)
			fatalx(errno, "Failed to read pancake number %d\n", i);
	}
}

Pancake::State Pancake::initstate(void) {
	State s;

	for (unsigned int i = 0; i < Ncakes; i++)
		s.cakes[i] = init[i];
	s.h = ngaps(s.cakes);

	return s;
}