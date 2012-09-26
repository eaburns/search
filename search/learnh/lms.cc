#include "../../utils/utils.hpp"
#include "../../rdb/rdb.hpp"
#include <cassert>

// Sorry, this is some of the ugliest code
// that I have ever written.

// If this is not the empty string then spt-it-out
// code showing the least squares fit is dumped
// to the given file.
std::string sptpath = "lms.spt";

extern "C" void dgels_( char* trans, int* m, int* n, int* nrhs, double* a, int* lda, double* b, int* ldb, double* work, int* lwork, int* info );

// h, g, d, D
enum { NFeatures = 4 };

// A is the column-major array of features
// from the sample of instances, and b is
// a vector of target h* values.
double *A, *b;

// Acopy is a copy of A which is trashed by dgels.
// If sptpath == "" then Acopy is not used.
double *Acopy;

// Hstar is a copy of b, because dgels trashes b.
// If sptpath == "" then hstar is not used.
double *hstar;

// Filled marks the rows of A that have been
// filled.  Just used for a sanity check.
bool *filled;

// sz is the sample size.
unsigned long sz;

// t is the count of records seen so far.
// It is used by the reservoire sampling.
unsigned long t;

// setrow sets the rth row of the A matrix to h,g,d,D
static void setrow(unsigned int r, double h, double g, double d, double D);

static void dfline(std::vector<std::string>&, void*);
static void sptheader(FILE*);
static void sptpoints(FILE*);
static void sptfooter(FILE*);

int main(int argc, const char *argv[]) {
	if (argc < 2)
		fatal("usage: lms <sample size> <rdbroot> [key=value*]");

	char *end = NULL;
	sz = strtol(argv[1], &end, 10);
	if (end == argv[1])
		fatal("Invalid sample size: %s", argv[1]);

	A = new double[NFeatures * sz];
	b = new double[sz];
	if (sptpath != "") {
		Acopy = new double[NFeatures * sz];
		hstar = new double[sz];
	}

	filled = new bool[sz];
	for (unsigned int i = 0; i < sz; i++)
		filled[i] = false;

	const char *root = argv[2];
	RdbAttrs attrs = attrargs(argc-3, argv+3);

	fprintf(stderr, "sample size: %lu\n", sz);
	fprintf(stderr, "root: %s\n", root);
	fputs(attrs.string().c_str(), stderr);
	fputc('\n', stderr);

	auto paths = withattrs(root, attrs);
	for (auto p = paths.begin(); p != paths.end(); p++) {
		FILE *f = fopen(p->c_str(), "r");
		if (!f) {
			warnx(errno, "Failed to open %s for reading", p->c_str());
			continue;
		}
		dfread(f, dfline);
		fclose(f);
	}

	fprintf(stderr, "%lu solutions\n", t);

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

	int n = NFeatures;
	int nrhs = 1;
	int lda = m;
	int ldb = m;
	int info;
	double wksz, *work;

	// Get a good work size.
	int lwork = -1;
	char trans[] = "No transpose";
	dgels_(trans, &m, &n, &nrhs, A, &lda, b, &ldb, &wksz,
		&lwork, &info);
	lwork = (int) wksz;
	fprintf(stderr, "got lwork: %d\n", lwork);

	work = new double[lwork];

	// Solve.
	dgels_(trans, &m, &n, &nrhs, A, &lda, b, &ldb, work,
		&lwork, &info);
	if (info > 0)
		fatal("info=%d", info);

	sptpoints(sptfile);
	fprintf(stderr, "h=%g\ng=%g\nd=%g\nD=%g\n", b[0], b[1], b[2], b[3]);
	printf( "%6.2f;%6.2f;%6.2f;%6.2f\n", b[0], b[1], b[2], b[3]);

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

	double g = strtod(l[2].c_str(), NULL);
	double h = strtod(l[3].c_str(), NULL);
	double D = strtod(l[4].c_str(), NULL);
	double d = strtod(l[5].c_str(), NULL);
	setrow(i, h, g, d, D);

	b[i] = strtod(l[6].c_str(), NULL);
	if (hstar)
		hstar[i] = b[i];
}

static void setrow(unsigned int r, double h, double g, double d, double D) {
	assert (r + 3*sz < NFeatures*sz);
	A[r+0*sz] = h;
	A[r+1*sz] = g;
	A[r+2*sz] = d;
	A[r+3*sz] = D;
	if (Acopy) {
		Acopy[r+0*sz] = h;
		Acopy[r+1*sz] = g;
		Acopy[r+2*sz] = d;
		Acopy[r+3*sz] = D;
	}
	filled[r] = true;
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
		double htrue = hstar[i];
		double h = Acopy[i+0*sz];
		double g = Acopy[i+1*sz];
		double d = Acopy[i+2*sz];
		double D = Acopy[i+3*sz];
		double hest = h*b[0] + g*b[1] + d*b[2] + D*b[3];
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