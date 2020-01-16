// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "../../rdb/rdb.hpp"
#include "../../utils/utils.hpp"
#include <map>
#include <limits>
#include <cerrno>
#include <cmath>

static double wf, wt;
static std::map<double, unsigned long> nsols;
static std::map<double, double> avg;

static void dfline(std::vector<std::string>&, void*);

struct Fields {
	double weight, cost, time;
	const char *path;
};

int main(int argc, const char *argv[]) {
	if (argc < 2)
		fatal("usage: bestwt <wf> <wt> <rdb root> [key=value*]");

	char *end = NULL;
	wf = strtod(argv[1], &end);
	if (end == argv[1])
		fatal("%s is an invalid wt value", argv[1]);

	wt = strtod(argv[2], &end);
	if (end == argv[2])
		fatal("%s is an invalid wt value", argv[2]);

	const char *root = argv[3];
	RdbAttrs attrs = attrargs(argc-4, argv+4);

	std::vector<std::string> paths = withattrs(root, attrs);
	fprintf(stderr, "%lu paths\n", (unsigned long) paths.size());
	for (auto p = paths.begin(); p != paths.end(); p++) {
		FILE *f = fopen(p->c_str(), "r");
		if (!f) {
			warnx(errno, "failed to open %s for reading", p->c_str());
			continue;
		}

		Fields fields;
		fields.weight = std::numeric_limits<double>::infinity();
		fields.cost = std::numeric_limits<double>::infinity();
		fields.time = std::numeric_limits<double>::infinity();
		fields.path = p->c_str();
		dfread(f, dfline, &fields, NULL);

		if (std::isinf(fields.weight))
			fatal("%s: missing or infinite weight", p->c_str());

		if (std::isinf(fields.cost))
			fatal("%s: missing or infinite cost", p->c_str());

		if (std::isinf(fields.time))
			fatal("%s: missing or infinite time", p->c_str());

		nsols[fields.weight]++;
		double cur = avg[fields.weight];
		double val = wf*fields.cost + wt*fields.time;
		avg[fields.weight] += (val - cur) / nsols[fields.weight];

		fclose(f);
	}

	double weight = avg.begin()->first;
	for (auto i = avg.begin(); i != avg.end(); i++) {
		if (i->second < avg[weight])
			weight = i->first;
	}

	printf("%g\n", weight);
}

static void dfline(std::vector<std::string> &line, void *aux) {
	char *end = NULL;
	const char *str;

	Fields *fields = static_cast<Fields*>(aux);

	if (line[0] == "#pair" && line[1] == "weight") {
		str = line[2].c_str();
		fields->weight = strtod(str, &end);
		if (end == str)
			fatal("%s: %s is not a valid weight", fields->path, str);
	} else if (line[0] == "#pair" && line[1] == "final sol cost") {
		str = line[2].c_str();
		fields->cost = strtod(str, &end);
		if (end == str)
			fatal("%s: %s is not a valid cost", fields->path, str);
	} else if (line[0] == "#pair" && line[1] == "total wall time") {
		str = line[2].c_str();
		fields->time = strtod(str, &end);
		if (end == str)
			fatal("%s: %s is not a valid time", fields->path, str);
	}
}