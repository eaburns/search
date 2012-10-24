#include "utils.hpp"

bool test_commas();
bool test_doubleeq();
bool test_doubleneq();
bool test_point_angle();
bool test_line_isect();
bool test_line_isabove();
bool test_lineseg_midpt();
bool test_lineseg_along();
bool test_lineseg_length();
bool test_lineseg_isect();
bool test_lineseg_hits();
bool test_poly_contains();
bool test_poly_isects();
bool test_poly_minisect();
bool test_poly_hits();
bool test_base64enc();
bool test_base64dec();
bool test_encdec();
bool test_basename();
bool test_dirname();

static const Test tests[] = {
	Test("commas test", test_commas),
	Test("doubleeq test", test_doubleeq),
	Test("doubleneq test", test_doubleneq),
	Test("point angle test", test_point_angle),
	Test("line isect test", test_line_isect),
	Test("line isabove test", test_line_isabove),
	Test("lineseg midpt test", test_lineseg_midpt),
	Test("lineseg along test", test_lineseg_along),
	Test("lineseg length test", test_lineseg_length),
	Test("lineseg isect test", test_lineseg_isect),
	Test("lineseg hits test", test_lineseg_hits),
	Test("poly contains test", test_poly_contains),
	Test("poly isects test", test_poly_isects),
	Test("poly minisect test", test_poly_minisect),
	Test("poly hits test", test_poly_hits),
	Test("base64enc test", test_base64enc),
	Test("base64dec test", test_base64dec),
	Test("encode/decode test", test_encdec),
	Test("basename test", test_basename),
	Test("dirname test", test_dirname),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

void new_16_delete_bench(unsigned long, double*, double*);
void new_32_delete_bench(unsigned long, double*, double*);
void new_128_delete_bench(unsigned long, double*, double*);
void walltime_bench(unsigned long, double*, double*);
void cputime_bench(unsigned long, double*, double*);
void rand_bits_bench(unsigned long, double*, double*);
void rand_real_bench(unsigned long, double*, double*);
void rand_bench(unsigned long, double*, double*);
void ilog2_bench(unsigned long, double*, double*);
void log2_int_bench(unsigned long, double*, double*);
void log2_double_bench(unsigned long, double*, double*);
void sqrt_bench(unsigned long, double*, double*);
void atan2_bench(unsigned long, double*, double*);
void sin_bench(unsigned long, double*, double*);
void cos_bench(unsigned long, double*, double*);
void pow_bench(unsigned long, double*, double*);
void pow_ints_bench(unsigned long, double*, double*);
void ipow_bench(unsigned long, double*, double*);
void ceil_bench(unsigned long, double*, double*);
void floor_bench(unsigned long, double*, double*);

static const Benchmark benches[] = {
	Benchmark("new(16)/delete() benchmark", new_16_delete_bench),
	Benchmark("new(32)/delete() benchmark", new_32_delete_bench),
	Benchmark("new(128)/delete() benchmark", new_128_delete_bench),
	Benchmark("walltime benchmark", walltime_bench),
	Benchmark("cputime benchmark", cputime_bench),
	Benchmark("randgen.bits benchmark", rand_bits_bench),
	Benchmark("randgen.real benchmark", rand_real_bench),
	Benchmark("rand() benchmark", rand_bench),
	Benchmark("ilog2 benchmark", ilog2_bench),
	Benchmark("log2 with ints benchmark", log2_int_bench),
	Benchmark("log2 with doubles benchmark", log2_double_bench),
	Benchmark("sqrt benchmark", sqrt_bench),
	Benchmark("atan2 benchmark", atan2_bench),
	Benchmark("sin benchmark", sin_bench),
	Benchmark("cos benchmark", cos_bench),
	Benchmark("pow benchmark", pow_bench),
	Benchmark("pow ints benchmark", pow_ints_bench),
	Benchmark("ipow benchmark", ipow_bench),
	Benchmark("ceil benchmark", ceil_bench),
	Benchmark("floor benchmark", floor_bench),
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

unsigned int *randuints(unsigned long n) {
	static unsigned int *ints;
	static unsigned long nints;

	if (nints < n) {
		if (ints)
			delete[] ints;
		ints = new unsigned int[n];
		nints = n;
	}
	for (unsigned long i = 0;  i < n; i++)
		ints[i] = randgen.bits();

	return ints;
}

double *randdoubles(unsigned long n) {
	static double *ds;
	static unsigned long nds;

	if (nds < n) {
		if (ds)
			delete[] ds;
		ds = new double[n];
		nds = n;
	}
	for (unsigned long i = 0;  i < n; i++)
		ds[i] = randgen.real();

	return ds;
}