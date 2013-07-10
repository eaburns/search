// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "pancake.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	dfheader(stdout);
	Pancake d(stdin);
	search<Pancake>(d, argc, argv);
	dffooter(stdout);
	return 0;
}
