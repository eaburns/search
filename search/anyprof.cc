// anyprof builds an anytime profile

#include "../rdb/rdb.hpp"
#include "../utils/utils.hpp"
#include <cerrno>

typedef std::pair<double,double> Sol;
typedef std::vector<Sol> Solutions;

static void usage(int);
static Solutions *readsols(const std::string&, const RdbAttrs&);
static void dfline(std::vector<std::string>&, void*);
static Sol maxsol(const Solutions*);

int main(int argc, const char *argv[]) {
	if (argc < 4)
		usage(1);

	double csteps = strtod(argv[1], NULL);
	double tsteps = strtod(argv[2], NULL);

	Solutions *sols = readsols(argv[3], attrargs(argc-4, argv+4));
	printf("%lu solutions\n", (unsigned long) sols->size());

	Sol s = maxsol(sols);
	printf("max cost: %g\nmax time: %g\n", s.first, s.second);


	// Just to mark them as used for now.
	csteps = csteps;
	tsteps = tsteps;

	return 0;
}

// usage prints the usage information and exits with
// the given status.
static void usage(int res) {
	puts("Usage: anyprof <cost steps> <time steps> <rdb root> [<key>=<val>...]");
	exit(res);
}

// readsols returns the solution bound and time vector from
// the datafiles specified by the given attributes.
static Solutions *readsols(const std::string &root, const RdbAttrs &attrs) {
	std::vector<std::string> files = withattrs(root, attrs);
	Solutions *sols = new Solutions();

	for (unsigned int i = 0; i < files.size(); i++) {
		printf("%s\n", files[i].c_str());
		FILE *f = fopen(files[i].c_str(), "r");
		if (!f)
			fatalx(errno, "failed to open %s for reading", files[i].c_str());
		dfread(f, dfline, sols, false);
		fclose(f);
	}
	return sols;
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
static void dfline(std::vector<std::string> &toks, void *solsp) {
	if (toks.size() != 9 || toks[0] != "#altrow" || toks[1] != "sol")
		return;
	double bound = strtod(toks[6].c_str(), NULL);
	double time = strtod(toks[8].c_str(), NULL);
	Solutions *sols = static_cast<Solutions*>(solsp);
	sols->push_back(Sol(bound, time));
}

// maxsol returns the max bound and time.
static Sol maxsol(const Solutions *sols) {
	Sol m(0, 0);
	for (unsigned int i = 0; i < sols->size(); i++) {
		const std::pair<double, double> &s = sols->at(i);
		if (s.first > m.first)
			m.first = s.first;
		if (s.second > m.second)
			m.second = s.second;
	}
	return m;
}