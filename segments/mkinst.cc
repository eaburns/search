#include "segments.hpp"
#include "../utils/utils.hpp"
#include <cmath>
#include <cstdio>

using namespace geom2d;

enum {
	Nsegs = 2,
	Width = 100,
	Height = 100,
	Nangles = 24,
	Seed = 0,

	// number of steps in the random walk
	Nsteps = 100000,
};

const double MaxR = 5;
const double MinR = 0.5;

void mkinst(FILE*);

int main(int argc, char *argv[]) {
	mkinst(stdout);
	return 0;
}

void mkinst(FILE *out) {
	Segments dom(Width, Height, Nangles);

	std::vector<LineSg> lines;
	for (unsigned int i = 0; i < Nsegs; i++) {
	redo:
		Segments::Seg seg;
		seg.radius = randgen.real()*(MaxR-MinR) + MinR;
		seg.start.rot = randgen.integer(0, Nangles-1);
		seg.start.x = randgen.integer(1, Width - 2);
		seg.start.y = randgen.integer(1, Height - 2);

		LineSg line = dom.line(seg, seg.start);
		for (auto l = lines.begin(); l != lines.end(); l++) {
			if (l->hits(line))
				goto redo;
		}

		lines.push_back(line);
		dom.segs.push_back(seg);
	}

	Segments::State st = dom.initialstate();
	for (unsigned int i = 0; i < Nsteps; i++) {
		Segments::Operators ops(dom, st);
		if (ops.size() == 0)
			break;
		unsigned int n = randgen.integer(0, ops.size()-1);
		Segments::Edge e(dom, st, ops[n]);
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