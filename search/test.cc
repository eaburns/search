#include "../utils/utils.hpp"
#include <cstdlib>
#include <ctime>

bool closedlist_add_test();
bool closedlist_find_test();
bool closedlist_rm_test();
bool closedlist_find_rand_test();
bool closed_iter_test();


static const Test tests[] = {
	Test("closed list add test", closedlist_add_test),
	Test("closed list find test", closedlist_find_test),
	Test("closed list rm test", closedlist_rm_test),
	Test("closed list find rand test", closedlist_find_rand_test),
	Test("closed iter test", closed_iter_test),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

//static const Benchmark benches[] = {};

//enum { Nbenches = sizeof(benches) / sizeof(benches[0]) };

int main(int argc, const char *argv[]) {
	const char *regexp = ".*";
	if (argc == 2)
		regexp = argv[1];

	srand(time(NULL));

	bool ok = runtests(tests, Ntests, regexp);
//	runbenches(benches, Nbenches, regexp);

	return ok ? 0 : 1;
}