#include "../incl/utils.hpp"
#include "intpq.hpp"
#include <cstdlib>
#include <boost/optional.hpp>

struct Ops { static int prio(int i) { return i; } };

bool intpq_push_test(void) {
	bool res = true;
	Intpq<Ops, int> pq;

	for (unsigned int i = 0; i < 100; i++) {
		pq.push(rand() % 1000);
		if (pq.fill != i+1) {
			testpr("Expected fill of %d got %d\n", i+1, pq.fill);
			res = false;
		}
	}
 
	return res;
}

bool intpq_pop_test(void) {
	bool res = true;
	Intpq<Ops, int> pq;

	for (int i = 0; i < 100; i++)
		pq.push(rand() % 1000);

	boost::optional<int> minopt = pq.pop();
	if (!minopt) {
		testpr("Empty pop");
		res = false;
	}
	int min = *minopt;

	for (unsigned int i = 1; i < 100; i++) {
		boost::optional<int> iopt = pq.pop();
		if (!iopt) {
			testpr("Empty pop");
			res = false;
		}

		int m = *iopt;
		if (pq.fill != 100 - (i+1)) {
			testpr("Expected fill of %d got %d\n", 100-(i+1), pq.fill);
			res = false;
		}

		if (m < min) {
			testpr("%d came out after %d\n", m, min);
			res = false;
		}
		min = m;
	}
 
	return res;
}