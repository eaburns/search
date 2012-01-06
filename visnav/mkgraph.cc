// Reads in a polygon map and emits a visibility graph
#include "visgraph.hpp"
#include <cstdio>

int main(void) {
	PolyMap map(stdin);
	VisGraph graph(map.polys);
	graph.output(stdout);
	return 0;
}