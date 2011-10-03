#include "../utils/utils.hpp"
#include "stn.hpp"

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

	if (stn.lower(1) != Stn::neginf()) {
		testpr("Earliest time is not initially negitive infinity");
		return false;
	}

	if (stn.upper(1) != Stn::inf()) {
		testpr("Latest time is not initially infinity");
		return false;
	}

	if (!stn.add(Stn::NoEarlier(1, 1))) {
		testpr("Failed to add the no earlier constraint\n");
		return false;
	}

	if (stn.lower(1) != 1) {
		testpr("Earliest time is not 1 after no earlier constraint");
		return false;
	}

	if (stn.upper(1) != Stn::inf()) {
		testpr("Latest time is not infinity after no earlier constraint");
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

bool stn_undo_one_test(void) {
	Stn stn(1);
	Stn copy(stn);

	if (!stn.add(Stn::NoEarlier(1, 5))) {
		testpr("Failed to add the no earlier constraint\n");
		return false;
	}

	if (copy.lower(1) == 5) {
		testpr("Copy was modified by changing the Stn\n");
		return false;
	}

	if (stn.eq(copy)) {
		testpr("Stn is equal to its copy after adding a constraint\n");
		return false;
	}

	stn.undo();

	if (!stn.eq(copy)) {
		testpr("Stn is not equal to its copy after undo\n");
		return false;
	}

	return true;
}