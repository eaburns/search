#include "utils.hpp"

bool test_commas(void);
bool test_doubleeq(void);
bool test_doubleneq(void);
bool test_point_angle(void);
bool test_line_isect(void);
bool test_line_isabove(void);
bool test_lineseg_midpt(void);
bool test_lineseg_along(void);
bool test_lineseg_length(void);
bool test_lineseg_isect(void);
bool test_poly_contains(void);
bool test_poly_isects(void);
bool test_poly_minisect(void);
bool test_poly_hits(void);
bool test_rdbpathfor_newpath(void);
bool test_rdbpathfor_samepath(void);
bool test_rdbpathfor_shareprefix(void);
bool test_rdbpathfor_existing(void);
bool test_base64enc(void);
bool test_base64dec(void);
bool test_encdec(void);

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
	Test("poly contains test", test_poly_contains),
	Test("poly isects test", test_poly_isects),
	Test("poly minisect test", test_poly_minisect),
	Test("poly hits test", test_poly_hits),
	Test("rdbpathfor newpath test", test_rdbpathfor_newpath),
	Test("rdbpathfor samepath test", test_rdbpathfor_samepath),
	Test("rdbpathfor shareprefix test", test_rdbpathfor_shareprefix),
	Test("rdbpathfor existing test", test_rdbpathfor_existing),
	Test("base64enc test", test_base64enc),
	Test("base64dec test", test_base64dec),
	Test("encode/decode test", test_encdec),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

void malloc_16_free_bench(unsigned long, double*, double*);
void malloc_32_free_bench(unsigned long, double*, double*);
void malloc_128_free_bench(unsigned long, double*, double*);
void rand_bits_bench(unsigned long, double*, double*);
void rand_real_bench(unsigned long, double*, double*);
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
	Benchmark("malloc(16)/free() benchmark", malloc_16_free_bench),
	Benchmark("malloc(32)/free() benchmark", malloc_32_free_bench),
	Benchmark("malloc(128)/free() benchmark", malloc_128_free_bench),
	Benchmark("randgen.bits benchmark", rand_bits_bench),
	Benchmark("randgen.real benchmark", rand_real_bench),
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