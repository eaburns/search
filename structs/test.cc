#include "../incl/utils.hpp"

bool htable_add_test(void);
bool htable_find_test(void);
bool htable_find_rand_test(void);
bool intpqmin_push_test(void);
bool intpqmin_pop_test(void);
bool intpqmax_push_test(void);
bool intpqmax_pop_test(void);

static const Test tests[] = {
	Test("htable add test", htable_add_test),
	Test("htable find test", htable_find_test),
	Test("htable find rand test", htable_find_test),
	Test("intpqmin push test", intpqmin_push_test),
	Test("intpqmin pop test", intpqmin_pop_test),
	Test("intpqmax push test", intpqmax_push_test),
	Test("intpqmax pop test", intpqmax_pop_test),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

void htable_add_bench(unsigned long n, double *, double *);
void htable_find_bench(unsigned long n, double *, double *);

static const Benchmark benches[] = {
	Benchmark("htable add benchmark", htable_add_bench),
	Benchmark("htable find benchmark", htable_find_bench),
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