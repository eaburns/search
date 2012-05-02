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
