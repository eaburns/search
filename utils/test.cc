#include "utils.hpp"

bool test_point_angle(void);
bool test_line_isect(void);
bool test_line_isabove(void);
bool test_lineseg_midpt(void);
bool test_lineseg_along(void);
bool test_lineseg_length(void);
bool test_lineseg_contains(void);
bool test_lineseg_isect(void);
bool test_poly_contains(void);
bool test_poly_isects(void);
bool test_poly_minisect(void);
bool test_poly_hits(void);
bool test_rdbpathfor_newpath(void);
bool test_rdbpathfor_samepath(void);
bool test_rdbpathfor_shareprefix(void);
bool test_rdbpathfor_existing(void);

static const Test tests[] = {
	Test("point angle", test_point_angle),
	Test("line isect", test_line_isect),
	Test("line isabove", test_line_isabove),
	Test("lineseg midpt", test_lineseg_midpt),
	Test("lineseg along", test_lineseg_along),
	Test("lineseg length", test_lineseg_length),
	Test("lineseg contains", test_lineseg_contains),
	Test("lineseg isect", test_lineseg_isect),
	Test("poly contains", test_poly_contains),
	Test("poly isects", test_poly_isects),
	Test("poly minisect", test_poly_minisect),
	Test("poly hits", test_poly_hits),
	Test("rdbpathfor newpath", test_rdbpathfor_newpath),
	Test("rdbpathfor samepath", test_rdbpathfor_samepath),
	Test("rdbpathfor shareprefix", test_rdbpathfor_shareprefix),
	Test("rdbpathfor existing", test_rdbpathfor_existing),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

static const Benchmark benches[] = {
//	Benchmark("htable add benchmark", htable_add_bench),
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