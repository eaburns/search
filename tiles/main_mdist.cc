#include "mdist.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	dfheader(stdout);
	TilesMdist d(stdin);
	search<TilesMdist>(d, argc, argv);
	dffooter(stdout);
	return 0;
}
