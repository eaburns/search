// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "scenario.hpp"
#include <iostream>

int main(int argc, const char *argv[]) {
	Scenario s(argc, argv);
	s.run(std::cin);
}