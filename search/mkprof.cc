// mkprof builds an anytime profile

#include "anyprof.hpp"
#include "../rdb/rdb.hpp"
#include "../utils/utils.hpp"
#include <limits>
#include <cerrno>
#include <cmath>

static void usage(int);
static void readsols(const std::string&, const RdbAttrs&);
static void dfline(std::vector<std::string>&, void*);
AnyProf profile(unsigned int, unsigned int);

struct Sol {
	Sol(double c, double t) : cost(c), time(t) { }

	double cost, time;
};

struct Inst {
	Inst(const std::string &f) : file(f), tmax(-1) { }

	std::string file;	
	std::vector<Sol> sols;
	double tmax;
};

static std::vector<Inst> insts;
static double tmax, cmax;

int main(int argc, const char *argv[]) {
	if (argc < 4)
		usage(1);

	std::string root = argv[3];
	unsigned int cbins = strtoul(argv[1], NULL, 10);
	unsigned int tbins = strtoul(argv[1], NULL, 10);

	RdbAttrs attrs = attrargs(argc-4, argv+4);
	if (!attrs.mem("alg"))
		fatal("must specify the alg attribute");

	readsols(root, attrs);
	AnyProf p = profile(cbins, tbins);

	RdbAttrs dstattrs;
	while (attrs.size() > 0) {
		std::string key = attrs.front();
		std::string vl = attrs.lookup(key);
		attrs.pop_front();
		if (key == "alg")
			vl += ".profile";
		dstattrs.push_back(key, vl);
	}

	std::string file = pathfor(root, dstattrs);
	printf("saving profile to %s\n", file.c_str());
	fflush(stdout);
	FILE *f = fopen(file.c_str(), "w");
	if (!f)
		fatalx(errno, "failed to open %s for writing", file.c_str());
	p.write(f);
	fclose(f);

	return 0;
}

// usage prints the usage information and exits with
// the given status.
static void usage(int res) {
	puts("Usage: mkprof <# cost bins> <# time bins> <rdb root> [<key>=<val>...]");
	exit(res);
}

static void readsols(const std::string &root, const RdbAttrs &attrs) {
	std::vector<std::string> files = withattrs(root, attrs);

	for (unsigned int i = 0; i < files.size(); i++) {
//		printf("%s\n", files[i].c_str());
		FILE *f = fopen(files[i].c_str(), "r");
		if (!f)
			fatalx(errno, "failed to open %s for reading", files[i].c_str());

		Inst inst(files[i]);
		dfread(f, dfline, &inst, false);
		fclose(f);

		if (inst.tmax < 0)
			fatal("no final solution time %s", inst.file.c_str());

		insts.push_back(inst);
	}
	printf("%lu datafiles\n", (unsigned long) files.size());
}

// 0: #altrow 
// 1: "sol"
// 2: "num"
// 3: "nodes expanded"
// 4: "nodes generated"
// 5: "weight"
// 6: "solution bound"
// 7: "solution cost"
// 8: "wall time"
static void dfline(std::vector<std::string> &toks, void *aux) {
	Inst *inst = static_cast<Inst*>(aux);

	if (toks.size() == 3 && toks[1] == "total wall time") {
		inst->tmax = strtod(toks[2].c_str(), NULL);
		if (inst->tmax > tmax)
			tmax = inst->tmax;
		return;
	}

	if (toks.size() != 9 || toks[0] != "#altrow" || toks[1] != "sol")
		return;

	double c = strtod(toks[7].c_str(), NULL);
	if (c > cmax)
		cmax = c;

	double t = strtod(toks[8].c_str(), NULL);
	inst->sols.push_back(Sol(c, t));
}

AnyProf profile(unsigned int cbins, unsigned int tbins) {
	AnyProf p(cbins, cmax, tbins, tmax);

	// Accumulate counts
	for (unsigned int i = 0; i < insts.size(); i++) {
		const Inst &inst = insts[i];

		for (unsigned int j = 1; j < inst.sols.size(); j++) {
			double deltat = inst.sols[j].time - inst.sols[j-1].time;
			if (deltat < 0)
				fatal("%s has a negative time delta", inst.file.c_str());
			unsigned int dt = deltat / p.twidth;
			unsigned int c0 = inst.sols[j-1].cost / p.cwidth;
			unsigned int c1 = inst.sols[j].cost / p.cwidth;
			assert (dt < tbins);
			assert (c0 < cbins);
			assert (c1 < cbins);

			p.bins[c0][dt][c1]++;
			for (unsigned int t = 0; t < dt; t++)
				p.bins[c0][t][c0]++;
		}

		unsigned int cost = inst.sols[inst.sols.size()-1].cost / p.cwidth;
		if (inst.sols[inst.sols.size()-1].time > inst.tmax)
			fatal("%s finds a solution after the search is done", inst.file.c_str());

		assert (cost < tbins);
		for (unsigned int dt = 0; dt < tbins; dt++)
			p.bins[cost][dt][cost]++;
	}

	// Normalize
	for (unsigned int c0 = 0; c0 < cbins; c0++) {
	for (unsigned int dt = 0; dt < tbins; dt++) {
		unsigned int s = 0;
		for (unsigned int c1 = 0; c1 < cbins; c1++)
			s += p.bins[c0][dt][c1];
		for (unsigned int c1 = 0; c1 < cbins; c1++)
			p.bins[c0][dt][c1] /= s;
	}
	}

	return p;
}
