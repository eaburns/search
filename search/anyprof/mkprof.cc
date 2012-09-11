#include "profile.hpp"
#include "../../rdb/rdb.hpp"
#include "../../utils/utils.hpp"
#include <cerrno>

static unsigned long nsols;

static void dfline(std::vector<std::string>&, void*);

int main(int argc, const char *argv[]) {
	if (argc < 4)
		fatal("usage: mkprof: <rdb root> <cost bins> <time bins> [key=value*]");

	const char *root = argv[1];
	printf("root: %s\n", root);

	errno = 0;
	char *end;
	unsigned int ncost = strtol(argv[2], &end, 10);
	if (end == argv[2])
		fatal("Invalid number of cost bins: %s", argv[2]);
	printf("%u cost bins\n", ncost);

	unsigned int ntime = strtol(argv[3], &end, 10);
	if (end == argv[3])
		fatal("Invalid number of time bins: %s", argv[3]);
	printf("%u time bins\n", ntime);

	RdbAttrs attrs = attrargs(argc-4, argv+4);
	printf("attributes:\n");
	for (auto k = attrs.getkeys().begin(); k != attrs.getkeys().end(); k++)
		printf("\t%s=%s\n", k->c_str(), attrs.lookup(*k).c_str());
	if (!attrs.mem("alg"))
		fatal("alg attribute must be specified");

	std::vector<std::string> paths = withattrs(root, attrs);
	printf("%lu data files\n", paths.size());

	std::vector<AnytimeProfile::SolutionStream> stream;
	for (auto p = paths.begin(); p != paths.end(); p++) {
		FILE *f = fopen(p->c_str(), "r");
		if (!f) {
			warnx(errno, "failed to open %s for reading", p->c_str());
			continue;
		}

		AnytimeProfile::SolutionStream sols;
		dfread(f, dfline, &sols, NULL);
		stream.push_back(sols);

		fclose(f);
	}
	printf("%lu solutions\n", nsols);
	if (nsols == 0) {
		puts("No solutions");
		return 1;
	}

	AnytimeProfile prof(ncost, ntime, stream);

	std::string alg = attrs.lookup("alg");
	attrs.rm("alg");
	attrs.push_back("alg", alg+".profile");
	attrs.push_back("cost bins", argv[2]);
	attrs.push_back("time bins", argv[3]);
	std::string opath = pathfor(root, attrs);
	printf("saving profile to %s\n", opath.c_str());

	FILE *f = fopen(opath.c_str(), "w");
	if (!f)
		fatal("failed to open %s for writing", opath.c_str());
	prof.save(f);
	fclose(f);

	return 0;
}

static void dfline(std::vector<std::string> &line, void *aux) {
	AnytimeProfile::SolutionStream *sols = static_cast<AnytimeProfile::SolutionStream*>(aux);

	// sol is the deprecated altrow name for ARA*
	// incumbent solutions, incumbent is the new
	// name.
	if (line[0] != "#altrow" || (line[1] != "sol" && line[1] != "incumbent"))
		return;

	AnytimeProfile::Solution sol;
	sol.cost = strtod(line[7].c_str(), NULL);
	sol.time = strtod(line[8].c_str(), NULL);
	sols->push_back(sol);
	nsols++;
}