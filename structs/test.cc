#include "../utils/utils.hpp"
#include <cstdlib>
#include <ctime>

bool htable_add_test(void);
bool htable_find_test(void);
bool htable_find_rand_test(void);
bool intpq_push_test(void);
bool intpq_pop_test(void);
bool binheap_push_test(void);
bool binheap_pop_test(void);
bool stn_copy_eq_test(void);
bool stn_add_one_ok_test(void);
bool stn_undo_one_test(void);

static const Test tests[] = {
	Test("htable add test", htable_add_test),
	Test("htable find test", htable_find_test),
	Test("htable find rand test", htable_find_test),
	Test("intpq push test", intpq_push_test),
	Test("intpq pop test", intpq_pop_test),
	Test("binheap push test", binheap_push_test),
	Test("binheap pop test", binheap_pop_test),
	Test("stn copy/eq test", stn_copy_eq_test),
	Test("stn add one ok test", stn_add_one_ok_test),
	Test("stn undo one test", stn_undo_one_test),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

void htable_add_bench(unsigned long, double *, double *);
void htable_find_bench(unsigned long, double *, double *);
void intpq_push_bench(unsigned long, double *, double *);
void intpq_pop_bench(unsigned long, double *, double *);
void binheap_push_bench(unsigned long, double*, double*);
void binheap_pop_bench(unsigned long, double*, double*);
void stn_add_bench(unsigned long, double*, double*);
void stn_undo_bench(unsigned long, double*, double*);

static const Benchmark benches[] = {
	Benchmark("htable add benchmark", htable_add_bench),
	Benchmark("htable find benchmark", htable_find_bench),
	Benchmark("intpq push benchmark", intpq_push_bench),
	Benchmark("intpq pop benchmark", intpq_pop_bench),
	Benchmark("binheap push benchmark", binheap_push_bench),
	Benchmark("binheap pop benchmark", binheap_pop_bench),
	Benchmark("stn add benchmark", stn_add_bench),
//	Benchmark("stn undo benchmark", stn_undo_bench),
};

enum { Nbenches = sizeof(benches) / sizeof(benches[0]) };

int main(int argc, const char *argv[]) {
	const char *regexp = ".*";
	if (argc == 2)
		regexp = argv[1];

	srand(time(NULL));

	bool ok = runtests(tests, Ntests, regexp);
	runbenches(benches, Nbenches, regexp);

	return ok ? 0 : 1;
}