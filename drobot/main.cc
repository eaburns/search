#include "drobot.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	DockRobot d(stdin);
	search<DockRobot>(d, argc, argv);
	return 0;
}