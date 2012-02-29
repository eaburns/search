#include "../utils/utils.hpp"
#include "stn.hpp"
#include <cstdlib>

bool stn_copy_eq_test(void) {
	Stn stn(1);
	Stn copy(stn);
	Stn stn2(2);

	if (!stn.eq(copy)) {
		stn.output(stderr);
		fprintf(stderr, "\n\n");
		copy.output(stderr);
		testpr("Stn is not equal to its copy\n");
		return false;
	}

	if (stn.eq(stn2)) {
		testpr("Stn is equal to a different Stn\n");
		return false;
	}

	return true;
}

bool stn_add_one_ok_test(void) {
	Stn stn(1);

	if (!stn.add(Stn::NoEarlier(1, 1))) {
		testpr("Failed to add the no earlier constraint\n");
		return false;
	}

	if (stn.lower(1) != 1) {
		testpr("Earliest time is not 1 after no earlier constraint");
		return false;
	}

	if (!stn.add(Stn::NoLater(1, 5))) {
		testpr("Failed to add the no later constraint\n");
		return false;
	}

	if (stn.lower(1) != 1) {
		testpr("Earliest time is not 1 after both constraints");
		return false;
	}

	if (stn.upper(1) != 5) {
		testpr("Latest time is not 5 after both constraints");
		return false;
	}

	return true;
}

bool stn_add_one_incons_test(void) {
	Stn stn(1);

	if (!stn.add(Stn::NoEarlier(1, 5))) {
		testpr("Failed to add the no earlier constraint\n");
		return false;
	}

	if (stn.lower(1) != 5) {
		testpr("Earliest time is not 5 after no earlier constraint");
		return false;
	}

	Stn copy(stn);

	if (stn.add(Stn::NoLater(1, 1))) {
		testpr("Successfully added the inconsistent no later constraint\n");
		return false;
	}

	if (!copy.eq(stn)) {
		testpr("Stn was not properly restored after adding the inconsistent constraint\n");
		return false;
	}

	return true;
}

bool stn_undo_one_test(void) {
	Stn stn(1);
	Stn copy(stn);

	if (!stn.add(Stn::NoEarlier(1, 5))) {
		testpr("Failed to add the no earlier constraint\n");
		return false;
	}

	if (stn.eq(copy)) {
		testpr("Stn is equal to its copy after adding a constraint\n");
		return false;
	}

	stn.undo();

	if (!stn.eq(copy)) {
		stn.output(stderr);
		copy.output(stderr);
		testpr("Stn is not equal to its copy after undo\n");
		return false;
	}

	return true;
}

enum { Nnodes = 1024 };

Stn::Constraint *cs;

void stn_add_bench(unsigned long n, double *strt, double *end) {
	cs = new Stn::Constraint[n];
	for (unsigned long i = 0; i < n; i++) {
		cs[i].i = rand() % Nnodes;
		cs[i].j = rand() % Nnodes;
		cs[i].a = rand() & 1 ? rand() : -rand();
		cs[i].b = rand() & 1 ? rand() : -rand();
	}

	Stn stn(Nnodes);

	*strt = walltime();
	for (unsigned long i = 0; i < n; i++)
		stn.add(cs[i]);
	*end = walltime();

	delete[] cs;
}

void stn_undo_bench(unsigned long n, double *strt, double *end) {
	Stn stn(Nnodes);

	for (unsigned long i = 0; i < n;) {
		Stn::Constraint c(
			rand() % Nnodes,
			rand() % Nnodes,
			rand() & 1 ? rand() : -rand(),
			rand() & 1 ? rand() : -rand()
		);
		if (stn.add(c))
			i++;
	}


	*strt = walltime();
	for (unsigned long i = 0; i < n; i++)
		stn.undo();
	*end = walltime();
}