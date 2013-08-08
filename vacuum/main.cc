// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "vacuum.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	dfheader(stdout);
	Vacuum d(stdin);
	search<Vacuum>(d, argc, argv);
	dffooter(stdout);
	return 0;
}
