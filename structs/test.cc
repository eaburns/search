#include "../incl/utils.hpp"

bool htable_add_test(void);
bool htable_find_test(void);
bool htable_find_rand_test(void);
bool intpq_push_test(void);
bool intpq_pop_test(void);
bool binheap_push_test(void);
bool binheap_pop_test(void);

static const Test tests[] = {
	Test("htable add test", htable_add_test),
	Test("htable find test", htable_find_test),
	Test("htable find rand test", htable_find_test),
	Test("intpq push test", intpq_push_test),
	Test("intpq pop test", intpq_pop_test),
	Test("binheap push test", binheap_push_test),
	Test("binheap pop test", binheap_pop_test),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

void htable_add_bench(unsigned long n, double *, double *);
void htable_find_bench(unsigned long n, double *, double *);
void intpq_push_bench(unsigned long n, double *, double *);
void intpq_pop_bench(unsigned long n, double *, double *);
void binheap_push_bench(unsigned long n, double*, double*);
void binheap_pop_bench(unsigned long n, double*, double*);

static const Benchmark benches[] = {
	Benchmark("htable add benchmark", htable_add_bench),
	Benchmark("htable find benchmark", htable_find_bench),
	Benchmark("intpq push benchmark", intpq_push_bench),
	Benchmark("intpq pop benchmark", intpq_pop_bench),
	Benchmark("binheap push benchmark", binheap_push_bench),
	Benchmark("binheap pop benchmark", binheap_pop_bench),
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