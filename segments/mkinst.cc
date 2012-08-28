#include "segments.hpp"
#include "../utils/utils.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cassert>
using namespace geom2d;

static unsigned int Nsegs = 10;
static unsigned int Width = 100;
static unsigned int Height = 100;
static unsigned int Nangles = 32;
static unsigned long Nsteps = 100000;
static double MaxR = 15;
static double MinR = 1;
static unsigned long Seed = 0;

void parseargs(int, const char*[]);
void helpmsg(int);
void mkinst(FILE*);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);
	mkinst(stdout);
	return 0;
}

void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (i < argc-1 && strcmp(argv[i], "-nsegs") == 0) {
			Nsegs = strtol(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-width") == 0) {
			Width = strtol(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-height") == 0) {
			Height = strtol(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-nangles") == 0) {
			Nangles = strtol(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-steps") == 0) {
			Nsteps = strtoul(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-maxr") == 0) {
			MaxR = strtod(argv[++i], NULL);

		} else if (i < argc-1 && strcmp(argv[i], "-minr") == 0) {
			MinR = strtod(argv[++i], NULL);

		} else if (i < argc-1 && strcmp(argv[i], "-seed") == 0) {
			Seed = strtoul(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-h") == 0) {
			helpmsg(0);

		} else {
			helpmsg(1);
		}
	}
	if (Seed == 0)
		Seed = walltime() * 1e9;
}

void helpmsg(int status) {
	puts("Usage: mkinst [options]");
	printf("-nsegs	the number of segments (default: %u)\n", Nsegs);
	printf("-width	width of the grid (default: %u)\n", Width);
	printf("-height	height of the grid (default: %u)\n", Height);
	printf("-nangles	the number of angles (must be even, default: %u)\n", Nangles);
	printf("-nsteps	number of steps in the random walk (default: %lu)\n", Nsteps);
	printf("-maxr	the max segment radius (default: %g)\n", MaxR);
	printf("-minr	the min segment radius (default: %g)\n", MinR);
	printf("-seed	specify the random seed\n");
	exit(status);
}

void mkinst(FILE *out) {
	Rand rnd(Seed);
	printf("# seed: %lu\n", Seed);

	Segments dom(Width, Height, Nangles, std::vector<Segments::Seg>());

	std::vector<LineSg> lines;
	for (unsigned int i = 0; i < Nsegs; i++) {
	redoseg:
		Segments::Seg seg;
		double r = rnd.real()*(MaxR-MinR) + MinR;
		seg.radius = r;
		seg.start.rot = rnd.integer(0, Nangles-1);
		seg.start.x = rnd.integer(ceil(r), Width - ceil(r));
		seg.start.y = rnd.integer(ceil(r), Height - ceil(r));

		LineSg line = dom.line(seg, seg.start);
		for (auto l = lines.begin(); l != lines.end(); l++) {
			if (l->hits(line))
				goto redoseg;
		}
		for (int i = 0; i < 4; i++) {
			if (dom.bounds[i].hits(line))
				goto redoseg;
		}

		lines.push_back(line);
		dom.segs.push_back(seg);
	}

	Segments::State st = dom.initialstate();
	Segments::Oper rev = Segments::Nop;

	for (unsigned long i = 0; i < Nsteps; i++) {
		Segments::Operators ops(dom, st);
		if (ops.size() == 0) {
			warn("Only performed %lu steps", i);
			break;
		}
		unsigned int n = rnd.integer(0, ops.size()-1);
		if (ops[n] == rev && ops.size() > 2) {
			unsigned int m = (n + rnd.integer(1, ops.size()-2)) % ops.size();
			assert (m != n);
			n = m;
		}

		Segments::Edge e(dom, st, ops[n]);
		rev = e.revop;
		st = e.state;
	}

	fprintf(out, "%u %u %u\n", Width, Height, Nangles);
	fprintf(out, "%u\n", Nsegs);
	for (unsigned int i = 0; i < dom.segs.size(); i++) {
		const Segments::Pose &goal = st.poses[i];
		const Segments::Seg &sg = dom.segs[i];
		fprintf(out, "%g ", sg.radius);
		fprintf(out, "%u %u %u ", sg.start.x, sg.start.y, sg.start.rot);
		fprintf(out, "%u %u %u\n", goal.x, goal.y, goal.rot);
	}
}
