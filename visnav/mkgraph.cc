// Reads in a polygon map and emits a visibility graph
#include "visgraph.hpp"
#include <cstdio>

int main() {
	PolyMap map(stdin);
	VisGraph graph(map);
	graph.output(stdout);
	return 0;
}