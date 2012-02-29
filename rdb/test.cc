#include "../utils/utils.hpp"

bool test_rdbpathfor_newpath(void);
bool test_rdbpathfor_samepath(void);
bool test_rdbpathfor_shareprefix(void);
bool test_rdbpathfor_existing(void);

static const Test tests[] = {
	Test("rdbpathfor newpath test", test_rdbpathfor_newpath),
	Test("rdbpathfor samepath test", test_rdbpathfor_samepath),
	Test("rdbpathfor shareprefix test", test_rdbpathfor_shareprefix),
	Test("rdbpathfor existing test", test_rdbpathfor_existing),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

int main(int argc, const char *argv[]) {
	const char *regexp = ".*";
	if (argc == 2)
		regexp = argv[1];
	return runtests(tests, Ntests, regexp) ? 0 : 1;
}
