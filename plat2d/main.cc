#include "plat2d.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, char *argv[]) {
	Plat2d d(stdin);
	search<Plat2d>(d, argc, argv);
	return 0;
}