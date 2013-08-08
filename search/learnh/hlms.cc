// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "../../utils/utils.hpp"
#include "../../rdb/rdb.hpp"
#include "leastsquares.hpp"
#include <cassert>
#include <cstring>

// If this is not the empty string then spt-it-out
// code showing the least squares fit is dumped
// to the given file.
std::string sptpath = "lms.spt";

// h, g, d, D
enum { NFeatures = 4 };

// A is the column-major array of features
// from the sample of instances, and b is
// a vector of target h* values.
double *A, *b;

// x is the result of the linear regression.
double x[NFeatures];

// Either "d" or "h", depending whether we are
// learning the distance estimate or heuristic.
const char *dorh;

// Filled marks the rows of A that have been
// filled.  Just used for a sanity check.
bool *filled;

// sz is the sample size.
unsigned long sz;

// t is the count of records seen so far.
// It is used by the reservoire sampling.
unsigned long t;

static void dfline(std::vector<std::string>&, void*);
static void sptheader(FILE*);
static void sptpoints(FILE*);
static void sptfooter(FILE*);

int main(int argc, const char *argv[]) {
	if (argc < 2)
		fatal("usage: lms <sample size> <d|h> <rdbroot> [key=value*]");

	// Seed with zero.
	randgen = Rand(0);

	char *end = NULL;
	sz = strtol(argv[1], &end, 10);
	if (end == argv[1])
		fatal("Invalid sample size: %s", argv[1]);

	A = new double[NFeatures * sz];
	b = new double[sz];

	filled = new bool[sz];
	for (unsigned int i = 0; i < sz; i++)
		filled[i] = false;

	dorh = argv[2];
	if (strcmp(dorh, "d") != 0 && strcmp(dorh, "h") != 0)
		fatal("Expected d or h, got %s", dorh);

	const char *root = argv[3];
	RdbAttrs attrs = attrargs(argc-4, argv+4);

	fprintf(stderr, "sample size: %lu\n", sz);
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

	fprintf(stderr, "%lu data points\n", t);

	int m = t < sz ? t : sz;
	for (unsigned int i = 0; i < (unsigned int) m; i++) {
		if (!filled[i])
			fatal("Row %u of %lu was never set", i, m);
	}
	if (m < NFeatures)
		fatal("Require at least %u solutions", NFeatures);

	FILE *sptfile = NULL;
	if (sptpath != "") {
		sptfile = fopen(sptpath.c_str(), "w");
		if (!sptfile) {
			fatalx(errno, "Failed to open %s for writing",
				sptpath.c_str());
		}
	}
	sptheader(sptfile);

	leastsquares(m, NFeatures, A, b, x);

	sptpoints(sptfile);
	fprintf(stderr, "h=%g\ng=%g\nd=%g\nD=%g\n", x[0], x[1], x[2], x[3]);
	printf( "%f,%f,%f,%f\n", x[0], x[1], x[2], x[3]);

	sptfooter(sptfile);

	return 0;
}

static void dfline(std::vector<std::string> &l, void*) {
	if (l[0] != "#altrow" || l[1] != "path")
		return;

	t++;
	unsigned long i = 0;
	if (t-1 < sz)
		i = t-1;
	else
		i = randgen.integer(0, t);
	if (i >= sz)
		return;

	A[i*NFeatures + 0] = strtod(l[3].c_str(), NULL);
	A[i*NFeatures + 1] = strtod(l[2].c_str(), NULL);
	A[i*NFeatures + 2] = strtod(l[5].c_str(), NULL);
	A[i*NFeatures + 3] = strtod(l[4].c_str(), NULL);
	if (dorh[0] == 'h')
		b[i] = strtod(l[6].c_str(), NULL);
	else	// 'd'
		b[i] = strtod(l[7].c_str(), NULL);
	filled[i] = true;
}

static void sptheader(FILE *f) {
	if (!f)
		return;
	fputs("(let* (\n", f);
}

static void sptpoints(FILE *f) {
	if (!f)
		return;

	fputs("\t(points	(\n", f);
	int m = t < sz ? t : sz;
	for (unsigned int i = 0; i < (unsigned int) m; i++) {
		double htrue = b[i];
		double h = A[i*NFeatures+0];
		double g = A[i*NFeatures+1];
		double d = A[i*NFeatures+2];
		double D = A[i*NFeatures+3];
		double hest = h*x[0] + g*x[1] + d*x[2] + D*x[3];
		fprintf(f, "\t(%f %f)\n", htrue, hest);
	}
	fputs("\t))\n", f);
}

static void sptfooter(FILE *f) {
	if (!f)
		return;
	fputs("\t(scatter (scatter-dataset :points points))\n", f);
	fputs("\t(plot (num-by-num-plot\n", f);
	fputs("\t\t:x-label \"h*\"\n", f);
	fputs("\t\t:y-label \"hhat\"\n", f);
	fputs("\t\t:dataset scatter))\n", f);
	fputs(") (display plot))\n", f);
}