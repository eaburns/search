// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

// sptpolicy dumps spt-it-out code that draws a
// a monitoring policy as a scatter plot of Xs and
// Os, representing stop and go respectively.

#include "profile.hpp"
#include "../../utils/utils.hpp"
#include <cerrno>

int main(int argc, const char *argv[]) {
	if (argc != 4)
		fatal("usage: sptpolicy <profile> <wf> <wt>");

	char *end;

	double wf = strtod(argv[2], &end);
	if (end == argv[2])
		fatal("invalid cost weight: %s\n", argv[2]);

	double wt = strtod(argv[3], &end);
	if (end == argv[3])
		fatal("invalid time weight: %s\n", argv[3]);

	FILE *f = fopen(argv[1], "r");
	if (!f)
		fatalx(errno, "failed to open %s for reading", argv[1]);
	AnytimeProfile prof(f);
	fclose(f);

	double qwidth = (prof.maxcost - prof.mincost) / prof.ncost;
	double twidth = (prof.maxtime - prof.mintime) / prof.ntime;

	AnytimeMonitor mon(prof, wf, wt);
 	std::vector< std::pair<double, double> > stop;
 	std::vector< std::pair<double, double> > go;

	for (unsigned int q = 0; q < prof.ncost; q++) {
	for (unsigned int t = 0; t < prof.ntime; t++) {
		double cost = prof.mincost + (q*qwidth) + qwidth/2;
		double time = prof.mintime + (t*twidth) + twidth/2;

		auto pt = std::make_pair(time, cost);
		if (mon.stop(cost, time))
			stop.push_back(pt);
		else
			go.push_back(pt);
	}
	}

	puts("(let* (");

	puts("	(stop-pts (");
	for (auto pt = stop.begin(); pt != stop.end(); pt++)
		printf("\t\t(%g %g)\n", pt->first, pt->second);
	puts("	))");
	puts("	(stop-scatter (scatter-dataset");
	puts("		:color (color :r 255)");
	puts("		:glyph \"cross\"");
	puts("		:points stop-pts))");

	puts("	(go-pts (");
	for (auto pt = go.begin(); pt != go.end(); pt++)
		printf("\t\t(%g %g)\n", pt->first, pt->second);
	puts("	))");
	puts("	(go-scatter (scatter-dataset");
	puts("		:color (color :g 255)");
	puts("		:glyph \"ring\"");
	puts("		:points go-pts))");

	puts("	(plot (num-by-num-plot");
	puts("		:x-label \"time\"");
	puts("		:y-label \"cost\"");
	puts("		:dataset stop-scatter");
	puts("		:dataset go-scatter)))");
	puts("	(display plot))");

	return 0;
}