// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "segments.hpp"
#include "../search/main.hpp"
#include <cstdio>

int main(int argc, const char *argv[]) {
	dfheader(stdout);
	Segments segs(stdin);
	segs.prinitial(stdout);
	search<Segments>(segs, argc, argv);
	dffooter(stdout);
	return 0;
}
