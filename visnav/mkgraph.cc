// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

// Reads in a polygon map and emits a visibility graph
#include "visgraph.hpp"
#include <cstdio>

int main() {
	PolyMap map(stdin);
	VisGraph graph(map);
	graph.output(stdout);
	return 0;
}