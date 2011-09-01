#include "../incl/utils.hpp"

bool htable_add_test(void);
bool htable_find_test(void);
bool intpq_push_test(void);
bool intpq_pop_test(void);

static const Test tests[] = {
	Test("htable add test", htable_add_test),
	Test("htable find test", htable_find_test),
	Test("intpq push test", intpq_push_test),
	Test("intpq pop test", intpq_pop_test),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

static const Benchmark benches[] = {
};

enum { Nbenches = sizeof(benches) / sizeof(benches[0]) };

int main(int argc, const char *argv[]) {
	const char *regexp = ".*";
	if (argc == 2)
		regexp = argv[1];

	bool ok = runtests(tests, Ntests, regexp);
	runbenches(benches, Nbenches, regexp);

	return ok ? 0 : 1;
}