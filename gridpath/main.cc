#include "scenario.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	Scenario s(argc, argv);
	s.run(std::cin);
}