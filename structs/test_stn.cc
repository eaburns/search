#include "../utils/utils.hpp"
#include "stn.hpp"

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

	Stn::NoEarlier ne(1, 1);
	if (!stn.add(ne)) {
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

	Stn::NoLater nl(1, 5);
	if (!stn.add(nl)) {
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
	return false;
}

void stn_add_bench(unsigned long n, double *strt, double *end) {
}