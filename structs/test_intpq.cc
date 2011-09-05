#include "../incl/utils.hpp"
#include "intpq.hpp"
#include <cstdlib>
#include <boost/optional.hpp>

bool intpqmin_push_test(void) {
	bool res = true;
	Intpqmin<int> pq;

	for (unsigned int i = 0; i < 100; i++) {
		unsigned int p = rand() % 1000;
		pq.add(p, p);
		if (pq.fill != i+1) {
			testpr("Expected fill of %d got %d\n", i+1, pq.fill);
			res = false;
		}
	}
 
	return res;
}

bool intpqmin_pop_test(void) {
	bool res = true;
	Intpqmin<unsigned int> pq;

	for (int i = 0; i < 100; i++) {
		unsigned int p = rand() % 1000;
		pq.add(p, p);
	}

	boost::optional<unsigned int> minopt = pq.pop();
	if (!minopt) {
		testpr("Empty pop");
		res = false;
	}
	int min = *minopt;

	for (unsigned int i = 1; i < 100; i++) {
		boost::optional<unsigned int> iopt = pq.pop();
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

bool intpqmax_push_test(void) {
	bool res = true;
	Intpqmax<int> pq;

	for (unsigned int i = 0; i < 100; i++) {
		unsigned int p = rand() % 1000;
		pq.add(p, p);
		if (pq.fill != i+1) {
			testpr("Expected fill of %d got %d\n", i+1, pq.fill);
			res = false;
		}
	}
 
	return res;
}

bool intpqmax_pop_test(void) {
	bool res = true;
	Intpqmax<unsigned int> pq;

	for (int i = 0; i < 100; i++) {
		unsigned int p = rand() % 1000;
		pq.add(p, p);
	}

	boost::optional<unsigned int> maxopt = pq.pop();
	if (!maxopt) {
		testpr("Empty pop");
		res = false;
	}
	int max = *maxopt;

	for (unsigned int i = 1; i < 100; i++) {
		boost::optional<unsigned int> iopt = pq.pop();
		if (!iopt) {
			testpr("Empty pop");
			res = false;
		}

		int m = *iopt;
		if (pq.fill != 100 - (i+1)) {
			testpr("Expected fill of %d got %d\n", 100-(i+1), pq.fill);
			res = false;
		}

		if (m > max) {
			testpr("%d came out after %d\n", m, max);
			res = false;
		}
		max = m;
	}
 
	return res;
}