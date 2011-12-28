#include "closedlist.hpp"
#include "scenario.hpp"
#include <iostream>

int main(int argc, const char *argv[]) {
	Scenario s(argc, argv);
	s.run(std::cin);
}