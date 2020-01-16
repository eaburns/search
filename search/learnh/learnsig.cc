// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

// Learns the standard deviation in heuristic error using corrected heuristics from hlms.
//
// This is a mess, and we never used it for anything, it's just here in case we ever
// want to look at in the future for whatever reason.
//

#include "../../utils/utils.hpp"
#include "../../rdb/rdb.hpp"
#include "leastsquares.hpp"
#include <cassert>
#include <cerrno>
#include <cstring>
#include <map>
#include <cmath>
#include <vector>

// If this is not the empty string then spt-it-out code showing the least
// squares fit is dumped to the given file.
std::string sptpath = "sigma.spt";

// A is the column-major array of features (d, d², ..., d^n) from the sample
// of instances, and b is a vector of target σ values.
double *A, *b;

// x is the result of the linear regression (it is an array with deg+1 elements).
double *x;

// Deg is the polynomial degree.
unsigned int deg;

// Filled marks the rows of A that have been
// filled.  Just used for a sanity check.
bool *filled;

// sz is the sample size.
unsigned long sz;

std::map<double, std::vector<double> > ds;

static void dfline(std::vector<std::string>&, void*);

static void sptheader(FILE*);
static void spttruepoints(FILE*);
static void sptestpoints(FILE*);
static void sptfooter(FILE*);
static double sigma(std::vector<double>);

int main(int argc, const char *argv[]) {
	if (argc < 3)
		fatal("usage: learnsig <deg> <rdbroot> [key=value*]");

	// Seed with zero.
	randgen = Rand(0);

	char *end = NULL;
	deg = strtol(argv[1], &end, 10);
	if (end == argv[1])
		fatal("Invalid degree: %s", argv[1]);

	const char *root = argv[2];
	RdbAttrs attrs = attrargs(argc-3, argv+3);

	fprintf(stderr, "degree: %u\n", deg);
	fprintf(stderr, "root: %s\n", root);
	fputs(attrs.string().c_str(), stderr);
	fputc('\n', stderr);

	auto paths = withattrs(root, attrs);
	fprintf(stderr, "%lu data files\n", paths.size());
	for (auto p = paths.begin(); p != paths.end(); p++) {
		FILE *f = fopen(p->c_str(), "r");
		if (!f) {
			warnx(errno, "Failed to open %s for reading", p->c_str());
			continue;
		}
		dfread(f, dfline);
		fclose(f);
	}

	sz = ds.size();
	fprintf(stderr, "%lu data points\n", sz);

	A = new double[(deg+1) * sz];
	b = new double[sz];
	x = new double[deg+1];
	unsigned int i = 0;
	for (auto e : ds) {
		double d = 1;
		for (unsigned int j = 0; j <= deg; j++) {
			A[i*(deg+1) + j] = d;
			d *= e.first;
		}
		b[i] = sigma(e.second);
		i++;
	}

	// Remove ds with ≤ 15 error values.  They mess up the data.
	std::vector<double> torment;
	for (auto e : ds) {
		if (e.second.size() < 15)
			torment.push_back(e.first);
	}
	for (auto k : torment)
		ds.erase(k);

	if (sz < deg+1)
		fatal("Require at least %u solutions", deg+1);

	FILE *sptfile = NULL;
	if (sptpath != "") {
		sptfile = fopen(sptpath.c_str(), "w");
		if (!sptfile)
			fatalx(errno, "Failed to open %s for writing", sptpath.c_str());
	}
	sptheader(sptfile);

	leastsquares(sz, deg+1, A, b, x);

	spttruepoints(sptfile);
	sptestpoints(sptfile);
	sptfooter(sptfile);

	for (unsigned int i = 0; i <= deg; i++)
		fprintf(stderr, "d^%u=%g\n", i, x[i]);
	for (unsigned int i = 0; i <= deg; i++) {
		printf("%.15f", x[i]);
		if (i < deg)
			printf(",");
	}
	printf("\n");

	return 0;
}

static void dfline(std::vector<std::string> &l, void*) {
	if (l[0] != "#altrow" || l[1] != "path")
		return;

	// 0 = altrow
	// 1 = row name
	// 2 = g
	// 3 = h
	// 4 = D
	// 5 = d
	// 6 = h*
	// 7 = d*

	double d = strtod(l[5].c_str(), NULL);
	double hstar = strtod(l[6].c_str(), NULL);
	double h = strtod(l[3].c_str(), NULL);

	ds[d].push_back(hstar-h);
}

static void sptheader(FILE *f) {
	if (!f)
		return;
	fputs("(let* (\n", f);
}

static void spttruepoints(FILE *f) {
	if (!f)
		return;

	fputs("\t(truepoints	(\n", f);
	for (auto e : ds)
		fprintf(f, "\t(%f %f)\n", e.first, sigma(e.second));
	fputs("\t))\n", f);
}

static void sptestpoints(FILE *f) {
	if (!f)
		return;

	fputs("\t(estpoints	(\n", f);
	for (auto e : ds) {
		double d = e.first;
		double est = 0;
		double term = 1;
		for (unsigned int i = 0; i <= deg; i++) {
			est += term*x[i];
			term *= d;
		}
		fprintf(f, "\t(%f %f)\n", d, est);
	}
	fputs("\t))\n", f);
}

static void sptfooter(FILE *f) {
	if (!f)
		return;
	fputs("\t(truescatter (scatter-dataset :color (color :r 255) :name \"true\" :points truepoints))\n", f);
	fputs("\t(estscatter (scatter-dataset :name \"est\" :points estpoints))\n", f);
	fputs("\t(plot (num-by-num-plot\n", f);
	fputs("\t\t:x-label \"d\"\n", f);
	fputs("\t\t:y-label \"sigma\"\n", f);
	fputs("\t\t:dataset estscatter\n", f);
	fputs("\t\t:dataset truescatter))\n", f);
	fputs(") (display plot))\n", f);
}

static double sigma(std::vector<double> fs) {
	double mu = 0;
	for (auto f : fs)
		mu += f;
	mu /= fs.size();

	double v = 0;
	for (auto f : fs)
		v += (f - mu)*(f - mu);
	v /= fs.size();
	return sqrt(v);
}