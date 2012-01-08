#include "../utils/utils.hpp"
#include <cstring>

bool test_commas(void) {
	struct { const char *str; int num; } ints[] = {
		{ "1", 1 },
		{ "10", 10 },
		{ "100", 100 },
		{ "1,000", 1000 },
		{ "1,000,000", 1000000 },
	};
	struct { const char *str; double num; } doubles[] = {
		{ "1.100000", 1.1 },
		{ "10.010000", 10.01 },
		{ "100.001000", 100.001 },
		{ "1,000.000100", 1000.0001 },
		{ "1,000,000.000000", 1000000.0000001 },
	};

	bool ok = true;
	for (unsigned int i = 0; i < sizeof(ints)/sizeof(ints[0]); i++) {
		const char *str = commas("%d", ints[i].num).c_str();
		if (strcmp(str, ints[i].str) == 0)
			continue;
		testpr("expected [%s], got [%s]\n", ints[i].str, str);
		ok = false;
	}
	for (unsigned int i = 0; i < sizeof(doubles)/sizeof(doubles[0]); i++) {
		const char *str = commas("%f", doubles[i].num).c_str();
		if (strcmp(str, doubles[i].str) == 0)
			continue;
		testpr("expected [%s], got [%s]\n", doubles[i].str, str);
		ok = false;
	}

	return ok;
}