#include <boost/optional.hpp>
#include "../incl/utils.hpp"
#include "htable.hpp"

struct Int {
	unsigned int vl;
	Int(int i) : vl(i) {}
	unsigned long hash() { return vl; }
	bool operator==(Int &a) const { return a.vl == vl; }
};

bool htable_add_test(void) {
	bool res = true;
	Htable<Int, unsigned int> ht;

	if (ht.fill != 0) {
		testpr("Hash table fill is not initialized to zero\n");
		res = false;
	}

	for (unsigned int i = 0; i < 100; i++) {
		ht.add(Int(i), i);
		if (ht.fill != i+1) {
			testpr("Hash table fill is not %u after %u adds\n", i+1, i+1);
			res = false;
		}
	}

	return res;
}

bool htable_find_test(void) {
	bool res = true;
	Htable<Int, unsigned int> ht;

	for (unsigned int i = 0; i < 100; i++)
		ht.add(Int(i), i);

	for (unsigned int i = 0; i < 100; i++) {
		boost::optional<unsigned int> vl = ht.find(Int(i));
		if (!vl) {
			testpr("No value mapped to key %u\n", i);
			res = false;
		} else if (*vl != i) {
			testpr("Found value %u mapped to key %u\n", *vl, i);
			res = false;
		}
	}

	return res;
}
