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