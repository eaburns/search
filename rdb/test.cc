#include "../utils/utils.hpp"

bool test_pathfor_newpath();
bool test_pathfor_samepath();
bool test_pathfor_shareprefix();
bool test_pathfor_existing();

static const Test tests[] = {
	Test("pathfor newpath test", test_pathfor_newpath),
	Test("pathfor samepath test", test_pathfor_samepath),
	Test("pathfor shareprefix test", test_pathfor_shareprefix),
	Test("pathfor existing test", test_pathfor_existing),
};

enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

int main(int argc, const char *argv[]) {
	const char *regexp = ".*";
	if (argc == 2)
		regexp = argv[1];
	return runtests(tests, Ntests, regexp) ? 0 : 1;
}
