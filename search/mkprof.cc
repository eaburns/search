// mkprof builds an anytime profile

#include "anyprof.hpp"
#include "../rdb/rdb.hpp"
#include "../utils/utils.hpp"
#include <limits>
#include <cerrno>
#include <cmath>

static void usage(int);
static std::vector<Imp> *readsols(const std::string&, const RdbAttrs&);
static void dfline(std::vector<std::string>&, void*);

int main(int argc, const char *argv[]) {
	if (argc < 4)
		usage(1);

	double cbins = strtod(argv[1], NULL);
	double tbins = strtod(argv[2], NULL);

	std::vector<Imp> *imps = readsols(argv[3], attrargs(argc-4, argv+4));
	printf("%lu solution improvements\n", (unsigned long) imps->size());

	AnyProf p(*imps, cbins, tbins);

	return 0;
}

// usage prints the usage information and exits with
// the given status.
static void usage(int res) {
	puts("Usage: mkprof <# cost bins> <# time bins> <rdb root> [<key>=<val>...]");
	exit(res);
}

// Aux holds extra information to communicate between
// calls of dfline.
struct Aux {
	std::vector<Imp> *imps;
	double prev;
};

// readsols returns the solution improvement info for
// the datafiles matching the attribute list.
static std::vector<Imp> *readsols(const std::string &root, const RdbAttrs &attrs) {
	std::vector<std::string> files = withattrs(root, attrs);
	Aux aux = { new std::vector<Imp>() };

	for (unsigned int i = 0; i < files.size(); i++) {
//		printf("%s\n", files[i].c_str());
		FILE *f = fopen(files[i].c_str(), "r");
		if (!f)
			fatalx(errno, "failed to open %s for reading", files[i].c_str());
		aux.prev = std::numeric_limits<double>::infinity();
		dfread(f, dfline, &aux, false);
		fclose(f);
	}

	printf("%lu datafiles\n", (unsigned long) files.size());
	return aux.imps;
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
static void dfline(std::vector<std::string> &toks, void *auxp) {
	if (toks.size() != 9 || toks[0] != "#altrow" || toks[1] != "sol")
		return;
	double bound = strtod(toks[6].c_str(), NULL);
	double time = strtod(toks[8].c_str(), NULL);
	Aux *aux = static_cast<Aux*>(auxp);
	if (!isinf(aux->prev))
		aux->imps->push_back(Imp(aux->prev, bound, time));
	aux->prev = bound;
}
