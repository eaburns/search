#include "plat2d.hpp"
#include "../visnav/visgraph.hpp"
#include <vector>
#include <cstdio>

int main(void) {
	Lvl lvl(stdin);
	std::vector<Polygon> polys = lvl.polys();
	VisGraph vg(polys);
	vg.output(stdout);
	return 0;
}