#include "gridpath.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, char *argv[]) {
	GridPath d(stdin);
	search<GridPath>(d, argc, argv);
	return 0;
}