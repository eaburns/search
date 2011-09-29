#include "mdist.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, char *argv[]) {
	TilesMdist d(stdin);
	search<TilesMdist>(d, argc, argv);
	return 0;
}