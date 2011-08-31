#include "../incl/utils.hpp"

bool htable_add_test(void);

static const Test tests[] = {
	Test("htable add test", htable_add_test),
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