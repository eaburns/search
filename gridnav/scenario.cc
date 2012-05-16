#include "scenario.hpp"
#include "../utils/utils.hpp"
#include "gridnav.hpp"
#include "closedlist.hpp"
#include "../search/main.hpp"
#include <iostream>
#include <cmath>

static const float Eps = 0.01;

Scenario::Scenario(int ac, const char *av[]) :
		argc(ac), argv(av), maproot("./"), lastmap(NULL),
		entry(-1), nentries(0) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-maproot") == 0 && i < argc - 1) {
			maproot = argv[i+1];
			i++;
		} else if (strcmp(argv[i], "-entry") == 0 && i < argc - 1) {
			entry = strtol(argv[i+1], NULL, 10);
			i++;
		}
	}
	if (maproot[maproot.size()-1] != '/')
		maproot += '/';
}

Scenario::~Scenario() {
	if (lastmap)
		delete lastmap;
}

void Scenario::run(std::istream &in) {
	checkver(in);
	outputhdr(stdout);

	SearchAlgorithm<GridNav> *srch = getsearch<GridNav>(argc, argv);
	ScenarioEntry ent(*this);
	while (in >> ent) {
		nentries++;
		if (entry >= 0 && nentries - 1 != entry)
			continue;

		Result<GridNav> r = ent.run(nentries-1, srch);
		res.add(r);
		srch->reset();
	}

	res.output(stdout);
	dfpair(stdout, "number of entries", "%u", nentries);
}

void Scenario::checkver(std::istream &in) {
	float ver;
	std::string verstr;
	std::cin >> verstr >> ver;
	if (verstr != "version")
		fatal("Expected a version header");
	if (ver != 1.0)
		fatal("Version %g is unsupported.  Latest supported version is 1.0", ver);
}

void Scenario::outputhdr(FILE *out) {
	dfrowhdr(out, "run", 15,
		"num", "bucket", "width", "height", "start x", "start y",
		"finish x", "finish y", "optimal sol", "nodes expanded",
		"nodes generated", "sol cost", "sol length",
		"wall time", "cpu time");
}

GridMap *Scenario::getmap(std::string mapfile) {
	std::string path = maproot + mapfile; 
	if (!lastmap || lastmap->file != path) {
		if (lastmap)
			delete lastmap;
		lastmap = new GridMap(path);
	}
	return lastmap;
}

ScenarioEntry::ScenarioEntry(Scenario &s) : scen(s) { }

Result<GridNav> ScenarioEntry::run(unsigned int n, SearchAlgorithm<GridNav> *srch) {
	GridNav d(scen.getmap(mapfile), x0, y0, x1, y1);
	GridNav::State s0 = d.initialstate();

	srch->search(d, s0);
	Result<GridNav> &res = srch->res;
	GridNav::Cost cost = d.pathcost(res.path, res.ops);
	// Scenario file has 0-cost for no-path.  We use -1.
	if (fabsf((cost - (double) opt) > Eps && !(opt == 0 && cost == GridNav::Cost(-1))))
		fatal("Expected optimal cost of %g, got %g\n", opt, (double) cost);

	dfrow(stdout, "run", "uuuuuuuuguugugg",
		(unsigned long) n, (unsigned long) bucket, (unsigned long) w,
		(unsigned long) h, (unsigned long) x0, (unsigned long) y0,
		(unsigned long)  x1, (unsigned long) y1, opt, res.expd, res.gend,
		(double) cost, (unsigned long) res.path.size(), res.wallend - res.wallstrt,
		res.cpuend - res.cpustrt);

	return res;
}

std::istream &operator>>(std::istream &in, ScenarioEntry &s) {
	in >> s.bucket;
	in >> s.mapfile;
	in >> s.w >> s.h;
	in >> s.x0 >> s.y0;
	in >> s.x1 >> s.y1;
	in >> s.opt;
	return in;
}