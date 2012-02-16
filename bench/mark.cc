#include "../utils/utils.hpp"
#include <cstdio>
#include <vector>
#include <string>
#include <cerrno>
#include <cstring>
#include <cmath>
#include <dirent.h>
#include <sys/utsname.h>

struct Domain {
	std::string name, fmt;
};

const Domain doms[] = {
	{ "tiles", "./tiles/15md_solver %s < %s" },
	{ "pancake", "./pancake/50pancake_solver %s < %s" },
	{ "gridnav", "./gridnav/gridnav_solver %s < %s" },
	{ "plat2d", "./plat2d/plat2d_solver %s -lvl %s" },
};

struct Algorithm {
	std::string name, cmd;
};

const Algorithm algs[] = {
	{ "astar", "astar" },
	{ "greedy", "greedy" },
	{ "wastar", "wastar -wt 1" },
};

// Separate since it doesn't work well on all domains.
const Algorithm idastar = { "idastar", "idastar" };

enum {
	Ndoms = sizeof(doms) / sizeof(doms[0]),
	Nalgs = sizeof(algs) / sizeof(algs[0]),
};

// Result holds results from a run.
struct Result {
	Result(void) : time(-1), len(0), nruns(0), stdev(-1) { }

	double time;	// time or mean time
	unsigned int len;	// solution length
	unsigned int nruns;	// only for saved benchmarks
	double stdev;	// only for saved benchmarks
};

char mname[256];

static bool bench(const Domain&, const Algorithm&);
static std::string instpath(const Domain&, const std::string&);
static std::string benchpath(const Domain&, const Algorithm&,const std::string&);
static bool chkbench(const Domain&, const Algorithm&, const std::string&);
static void mkbench(const Domain&, const Algorithm&, const std::string&);
static Result readresult(FILE*);
static void dfline(std::vector<const char *>&, void*);
static Result run(const Domain&, const Algorithm&, const std::string&);

int main() {
	if (gethostname(mname, sizeof(mname)) == -1)
		fatalx(errno, "failed to get the machine name");

	printf("machine: %s\n", mname);
	for (unsigned int i = 0; i < Ndoms; i++) {
		printf("domain: %s\n", doms[i].name.c_str());
		for (unsigned int j = 0; j < Nalgs; j++) {
			printf("algorithm: %s\n", algs[j].name.c_str());
			if (!bench(doms[i], algs[j]))
				return 1;
		}
		if (doms[i].name == "tiles" || doms[i].name == "pancake") {
			printf("algorithm: %s\n", idastar.name.c_str());
			if (!bench(doms[i], idastar))
				return 1;
		}
	}

	return 0;
}

// bench benchmarks the given domain and algorithm,
// returning true if the benchmark matched previous
// results.
static bool bench(const Domain &dom, const Algorithm &alg) {
	const std::string dir = "bench/insts/" + dom.name;
	std::vector<std::string> insts = readdir(dir, false);

	for (unsigned int i = 0; i < insts.size(); i++) {
		std::string inst = insts[i];
		printf("instance: %s (%s)\n", inst.c_str(),
			benchpath(dom, alg, inst).c_str());

		if (fileexists(benchpath(dom, alg, inst))) {
			if (!chkbench(dom, alg, inst)) {
				printf("failed\n");
				return false;
			}
		} else {
			mkbench(dom, alg, inst);
		}
	}

	return true;
}

static std::string instpath(const Domain &dom, const std::string &inst) {
	return "bench/insts/" + dom.name + "/" + inst;
}

static std::string benchpath(const Domain &dom, const Algorithm &alg,
		const std::string &inst) {
	return "bench/data/" + std::string(mname) + "_" +
		dom.name + "_" + alg.name + "_" + inst;
}

// chkbench returns true if the current behavior is
// consistent with the benchmark.
static bool chkbench(const Domain &dom, const Algorithm &alg,
		const std::string &inst) {
	const std::string ipath = benchpath(dom, alg, inst);
	FILE *f = fopen(ipath.c_str(), "r");
	if (!f)
		fatalx(errno, "failed to open %s for reading", ipath.c_str());
	Result bench = readresult(f);
	fclose(f);

	double limhigh = bench.time * 1.05;
	double limlow = bench.time * 0.95;

	// Must pass 1 of 3 runs within the limit
	unsigned int ok = 0;
	for (unsigned int i = 0; i < 3 && ok < 1; i++) {
		Result r = run(dom, alg, inst);
		if (r.len != bench.len) {
			printf("	expected %u solution length\n", bench.len);
			continue;
		}
		if (r.time > limhigh) {
			printf("	expected ≤ %g seconds (mean: %gs)\n", limhigh, bench.time);
			continue;
		}
		if (r.time < limlow)
			printf("	better: %g ≤ %g\n", r.time, limlow);
		ok++;
	}

	return ok >= 1;
}

// mkbench makes a new benchmark file.
static void mkbench(const Domain &dom, const Algorithm &alg,
		const std::string &inst) {
	static const unsigned int N = 5;
	const std::string opath = benchpath(dom, alg, inst);
	printf("	making %s\n", opath.c_str());

	printf("	warmup run:	");
	run(dom, alg, inst);

	Result r[N];
	double meantime = 0;
	for (unsigned int i = 0; i < N; i++) {
		r[i] = run(dom, alg, inst);
		meantime += r[i].time;
	}
	meantime /= N;

	double vartime = 0;
	for (unsigned int i = 1; i < N; i++) {
		if (r[i].len != r[0].len)
			fatal("different lengths when solving %s %s %s",
				dom.name.c_str(), alg.name.c_str(), inst.c_str());
		double diff = r[i].time - meantime;
		vartime += diff * diff;
	}
	vartime /= N;
	double stdev = sqrt(vartime);

	printf("	%g avg. seconds, %g stdev\n", meantime, stdev);

	ensuredir(opath);
	FILE *o = fopen(opath.c_str(), "w");
	if (!o)
		fatalx(errno, "failed to open %s for writing", opath.c_str());

	dfpair(o, "machine name", "%s", mname);
	dfpair(o, "domain", "%s", dom.name.c_str());
	dfpair(o, "algorithm", "%s", alg.name.c_str());
	dfpair(o, "instance", "%s", inst.c_str());
	dfpair(o, "num runs", "%u", (unsigned int) N);
	dfpair(o, "mean wall time", "%g", meantime);
	dfpair(o, "stdev wall time", "%g", stdev);
	dfpair(o, "final sol length", "%u", (unsigned int) r[0].len);
	fclose(o);
}

// readresult reads the datafile from the stream
// and returns the result data.
static Result readresult(FILE *f) {
	Result res;
	dfread(f, dfline, &res);
	return res;
}

static void dfline(std::vector<const char *> &toks, void *_res) {
	Result *res = static_cast<Result*>(_res);
	if (strcmp(toks[1], "total wall time") == 0) {
		res->time = strtod(toks[2], NULL);
	} else if (strcmp(toks[1], "final sol length") == 0) {
		res->len = strtol(toks[2], NULL, 10);
	} else if (strcmp(toks[1], "mean wall time") == 0) {
		res->time = strtod(toks[2], NULL);
	} else if (strcmp(toks[1], "stdev wall time") == 0) {
		res->stdev = strtod(toks[2], NULL);
	} else if (strcmp(toks[1], "num runs") == 0) {
		res->nruns = strtol(toks[2], NULL, 10);
	}
}

// run runs the solver on the given instance using the
// given algorithm, returning the Result.
static Result run(const Domain &dom, const Algorithm &alg,
		const std::string &inst) {
	char cmd[1024];
	const std::string ipath = instpath(dom, inst);
	unsigned int res = snprintf(cmd, sizeof(cmd), dom.fmt.c_str(),
		alg.cmd.c_str(), ipath.c_str());
	if (res >= sizeof(cmd))
		fatal("mark.cc run(): buffer is too small");

	FILE *f = popen(cmd, "r");
	if (!f)
		fatal("failed to execute [%s]", cmd);
	Result r = readresult(f);
	pclose(f);

	printf("	%u sol length, %g seconds\n", r.len, r.time);
	return r;
}