// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

// sptcounts dumps spt-it-out code that draws a
// heatmap of the counts of solution costs and times.

#include "profile.hpp"
#include "../../utils/utils.hpp"
#include <cerrno>

int main(int argc, const char *argv[]) {
	if (argc != 2)
		fatal("usage: sptcounts <profile>");

	FILE *f = fopen(argv[1], "r");
	if (!f)
		fatalx(errno, "failed to open %s for reading", argv[1]);
	AnytimeProfile prof(f);
	fclose(f);

	puts("(let* (");
	puts("	(sols (");
	for (unsigned int q = 0; q < prof.ncost; q++) {
	for (unsigned int t = 0; t < prof.ntime; t++) {
		printf("\t\t(%u %u %u)\n", t, q, prof.qtcount(q, t));
	}
	}
	puts("	))");
	puts("	(heatmap (heatmap-dataset");
	puts("		:bin-size (1 1)");
	puts("		:triples sols))");
	puts("	(plot (num-by-num-plot");
	puts("		:x-label \"time\"");
	puts("		:y-label \"cost\"");
	puts("		:dataset heatmap)))");
	puts("	(display plot))");

	return 0;
}