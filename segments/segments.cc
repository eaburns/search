#include "segments.hpp"
#include <cmath>
#include<limits>
#include <sstream>

using namespace geom2d;

static int wrapind(int i, int n);
static bool slicecontains(const Arc&, const Pt&);

Segments::Oper Segments::Nop(Oper::None, 0, 0, 0);

Segments::Angle::Angle(double t) : theta(t), sine(sin(t)), cosine(cos(t)) {
}

bool Segments::Sweep::hits(const LineSg &line) const {
	for (unsigned int i = 0; i < narcs; i++) {
		if (arcs[i].hits(line) || slicecontains(arcs[i], line.p0))
			return true;
	}
	for (unsigned int i = 0; i < nlines; i++) {
		if (lines[i].hits(line))
			return true;
	}
	for (unsigned int i = 0; i < npolys; i++) {
		if (polys[i].contains(line.p0))
			return true;
	}
	return false;
}

// slicecontains returns true if the slice defined by the arc
// contains the given point.
static bool slicecontains(const Arc &a, const Pt &p) {
	double t = Pt::angle(Pt(p.x - a.c.x, p.y - a.c.y));
	double tmin = a.t0;
	double tmax = a.t0+a.dt;
	if (tmin > tmax) {
		tmin = a.t0+a.dt;
		tmax = a.t0;
	}
	return between(tmin, tmax, t) && Pt::distance(a.c, p) <= a.r;
}

Segments::Segments(FILE *in) {
	// chomp away comments.
	int c = fgetc(in);
	while (c == '#') {
		do { 
			c = fgetc(in);
		} while (c != '\n');
		c = fgetc(in);
	}
	ungetc(c, in);

	if (fscanf(in, "%u %u %u\n", &width, &height, &nangles) != 3)
		fatal("Failed to read the turning angle step\n");

	if (nangles % 2 != 0)
		fatal("Angle step must partition a circle into an even number of segments");

	dtheta = 2*M_PI / nangles;
	for (unsigned int i = 0; i < nangles; i++)
		angles.push_back(Angle(dtheta*i));

	bounds[0] = LineSg(Pt(0, 0), Pt(0, width));
	bounds[1] = LineSg(Pt(0, width), Pt(height, width));
	bounds[2] = LineSg(Pt(height, width), Pt(height, 0));
	bounds[3] = LineSg(Pt(height, 0), Pt(0, 0));

	unsigned int n;
	if (fscanf(in, "%u\n", &n) != 1)
		fatal("Failed to read the number of segments\n");

	for (unsigned int i = 0; i < n; i++) {
		Seg seg;
		if (fscanf(in, "%lg ", &seg.radius) != 1)
			fatal("Failed to read the %uth radius\n", i);
		if (fscanf(in, " %u %u %u", &seg.start.x, &seg.start.y, &seg.start.rot) != 3)
			fatal("Failed to read the %uth initial pose\n", i);
		if (fscanf(in, " %u %u %u", &seg.goal.x, &seg.goal.y, &seg.goal.rot) != 3)
			fatal("Failed to read the %uth goal pose\n", i);
		segs.push_back(seg);
	}
}


Segments::Segments(unsigned int w, unsigned int h, unsigned int t, const std::vector<Seg> &s) :
	width(w), height(h), nangles(t), segs(s) {

	if (nangles % 2 != 0)
		fatal("Angle step must partition a circle into an even number of segments");

	dtheta = 2*M_PI / t;
	for (unsigned int i = 0; i < nangles; i++)
		angles.push_back(Angle(dtheta*i));

	bounds[0] = LineSg(Pt(0, 0), Pt(w, 0));
	bounds[1] = LineSg(Pt(w, 0), Pt(w, h));
	bounds[2] = LineSg(Pt(w, h), Pt(0, h));
	bounds[3] = LineSg(Pt(0, h), Pt(w, 0));
}

void Segments::State::init(const Segments &dom, const std::vector<Pose> &ps) {
	nleft = 0;

	poses.resize(ps.size());
	lines.resize(ps.size());
	for (unsigned int i = 0; i < ps.size(); i++) {
		poses[i] = ps[i];
		lines[i] = dom.line(dom.segs[i], ps[i]);

		if (ps[i] != dom.segs[i].goal)
			nleft++;
	}
}

geom2d::LineSg Segments::line(const Seg &s, const Pose &p) const {
	const Angle &a0 = angles[p.rot];
	const Angle &a1 = angles[wrapind(p.rot + nangles/2, nangles)];
	Pt p0(p.x + s.radius*a0.cosine, p.y + s.radius*a0.sine);
	Pt p1(p.x + s.radius*a1.cosine, p.y + s.radius*a1.sine);
	return LineSg(p0, p1);
}

bool Segments::State::operator==(const State &o) const {
	for (unsigned int i = 0; i < poses.size(); i++) {
		if (poses[i] != o.poses[i])
			return false;
	}
	return x == o.x && y == o.y && nleft == o.nleft;
}

bool Segments::Oper::operator==(const Oper &o) const {
	if (o.op != op)
		return false;
	if (op == None)
		return true;
	else if (op == Rotate)
		return seg == o.seg && delta == o.delta;
	return seg == o.seg && delta == o.delta && dy == o.dy;
}

Segments::Oper Segments::Oper::reverse() const {
	if (op == None)
		return Nop;
	return Oper(op, seg, -delta, -dy);
}

Segments::Cost Segments::Oper::cost(const State &state) const {
	if (op == None)
		return 0.0;

	double c = 0;

	const Pose &p = state.poses[seg];
	if (state.x != p.x || state.y != p.y) {
		double x = (double)state.x - p.x;
		double y = (double)state.y - p.y;
		c += sqrt(x*x + y*y);
	}

	if (op == Rotate)
		c += 1.0;

	else if (op == Move)
		c += (delta == 0 || dy == 0) ? 1.0 : sqrt(2);

	return c;
}

Segments::Sweep Segments::Oper::sweep(const Segments &dom, const State &s) const {
	if (op == None)
		return Sweep();
	if (op == Rotate)
		return rotatesweep(dom, s);
	return movesweep(dom, s);
}

Segments::Sweep Segments::Oper::rotatesweep(const Segments &dom, const State &s) const {
	Sweep sweep;
	Pose p = s.poses[seg];

	sweep.narcs = 2;
	Angle a0 = dom.angles[p.rot];
	double r = dom.segs[seg].radius;
	Pt center(p.x, p.y);
	sweep.arcs[0] = Arc(center, r, a0.theta, delta*dom.dtheta);
	sweep.arcs[1] = Arc(center, r, M_PI+a0.theta, delta*dom.dtheta);

	sweep.nlines = 1;
	p.rot = wrapind(p.rot+delta, dom.nangles);
	sweep.lines[0] = dom.line(dom.segs[seg], p);
	return sweep;
}

Segments::Sweep Segments::Oper::movesweep(const Segments &dom, const State &s) const {
	Sweep sweep;
	Pose p0 = s.poses[seg];
	Pose p1 = p0;
	p1.x += delta;
	p1.y += dy;

	sweep.nlines = 3;
	sweep.lines[0] = dom.line(dom.segs[seg], p1);
	const LineSg &l0 = s.lines[seg];
	const LineSg &l1 = sweep.lines[0];
	sweep.lines[1] = LineSg(l0.p0, l1.p0);
	sweep.lines[2] = LineSg(l0.p1, l1.p1);

	sweep.npolys = 1;
	sweep.polys[0] = Poly(4,
		l0.p0.x, l0.p0.y,
		l1.p0.x, l1.p0.y,
		l1.p1.x, l1.p1.y,
		l0.p1.x, l0.p1.y);

	return sweep;
}

bool Segments::Oper::ok(const Segments &dom, const State &s) const {
	if (op == None)
		return true;

	Sweep swp = sweep(dom, s);
	for (unsigned int i = 0; i < 4; i++) {
		if (swp.hits(dom.bounds[i]))
			return false;
	}

	for (unsigned int i = 0; i < s.lines.size(); i++) {
		if (i == seg)
			continue;
		if (swp.hits(s.lines[i]))
			return false;
	}

	return true;
}

Segments::Operators::Operators(const Segments &dom, const State &s) {
	for (unsigned int i = 0; i < s.poses.size(); i++) {
		Oper cw(Oper::Rotate, i, -1, 0);
		if (cw.ok(dom, s))
			ops.push_back(cw);

		Oper ccw(Oper::Rotate, i, 1, 0);
		if (ccw.ok(dom, s))
			ops.push_back(ccw);

		for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
			if (dx == dy)
				continue;
			Oper mv(Oper::Move, i, dx, dy);
			if (mv.ok(dom, s))
				ops.push_back(mv);
		}
		}
	}
}

Segments::Edge::Edge(const Segments &dom, const State &s, Oper op) {
	cost = op.cost(s);
	revop = op.reverse();
	state = s;

	if (op.op == Oper::None)
		return;

	Pose &p = state.poses[op.seg];

	if (op.op == Oper::Rotate) {
		p.rot = wrapind(p.rot+op.delta, dom.nangles);
		state.lines[op.seg] = dom.line(dom.segs[op.seg], p);

	} else if (op.op == Oper::Move) {
		p.x += op.delta;
		p.y += op.dy;
		state.lines[op.seg] = dom.line(dom.segs[op.seg], p);
	}

	// Move the gripper with the segment.
	state.x = p.x;
	state.y = p.y;

	bool wasgoal = s.poses[op.seg] == dom.segs[op.seg].goal;
	bool isgoal = p == dom.segs[op.seg].goal;
	if (wasgoal && !isgoal)
		state.nleft++;
	else if (!wasgoal && isgoal)
		state.nleft--;
}

Segments::State Segments::initialstate() const {
	std::vector<Pose> poses;
	for (auto sg = segs.begin(); sg != segs.end(); sg++)
		poses.push_back(sg->start);
	return State(*this, poses, 0, 0);
}

unsigned long Segments::hash(const PackedState &pkd) const {
	unsigned int ss[3*pkd.poses.size() + 2];
	unsigned int n = 0;
	ss[n++] = pkd.x;
	ss[n++] = pkd.y;
	for (auto p = pkd.poses.begin(); p != pkd.poses.end(); p++) {
		ss[n++] = p->rot;
		ss[n++] = p->x;
		ss[n++] = p->y;
	}
	return hashbytes((unsigned char*)ss, n*sizeof(ss[0]));
}

Segments::Cost Segments::h(const State &s) const {
	if (s.nleft == 0)
		return 0;

	double h = 0;
	double gripdist = std::numeric_limits<double>::infinity();
	for (unsigned int i = 0; i < s.poses.size(); i++) {
		if (s.poses[i] == segs[i].goal)
			continue;

		double dx = abs((int)segs[i].goal.x - (int)s.poses[i].x);
		double dy = abs((int)segs[i].goal.y - (int)s.poses[i].y);
		double shorter = dx < dy ? dx : dy;
		double longer = dx > dy ? dx : dy;
		h += (longer-shorter) + shorter*sqrt(2.0);

		int adist = wrapind(segs[i].goal.rot - s.poses[i].rot, nangles);
		int bdist = wrapind(s.poses[i].rot - segs[i].goal.rot, nangles);
		h += adist < bdist ? adist : bdist;

		dx = (double)s.x - s.poses[i].x;
		dy = (double)s.x - s.poses[i].y;
		double d = dx*dx + dy*dy;
		if (d < gripdist)
			gripdist = d;
	}

	return h + sqrt(gripdist);
}

Segments::Cost Segments::d(const State &s) const {
	if (s.nleft == 0)
		return 0;

	double d = 0;
	bool gripon = false;
	for (unsigned int i = 0; i < s.poses.size(); i++) {
		if (s.poses[i] == segs[i].goal)
			continue;

		double dx = labs((double)segs[i].goal.x - s.poses[i].x);
		double dy = labs((double)segs[i].goal.y - s.poses[i].y);
		d += dx > dy? dx : dy;

		int adist = wrapind(segs[i].goal.rot - s.poses[i].rot, nangles);
		int bdist = wrapind(s.poses[i].rot - segs[i].goal.rot, nangles);
		d += adist < bdist ? adist : bdist;

		if (s.x == s.poses[i].x && s.y == s.poses[i].y)
			gripon = true;
	}

	return d + (gripon ? 0 : 1);
}

Segments::Cost Segments::pathcost(const std::vector<State> &path, const std::vector<Oper> &ops) {
	State state = initialstate();
	Cost cost(0);

	if (ops.size() > (unsigned int) std::numeric_limits<int>::max())
		fatal("Too many actions");

	for (int i = ops.size() - 1; i >= 0; i--) {
		State copy(state);
		Edge e(*this, copy, ops[i]);
		cost += e.cost;
		assert(e.state == path[i]);
		state = e.state;
	}
	assert (isgoal(state));

	std::stringstream s;
	for (int i = ops.size() - 1; i >= 0; i--) {
		if (ops[i].op == Oper::Rotate)
			s << "r " << ops[i].seg << " " << ops[i].delta;

		else if (ops[i].op == Oper::Move)
			s << "m " << ops[i].seg << " " << ops[i].delta << " " << ops[i].dy;

		else
			fatal("Invalid path operator: %d\n", ops[i].op);
	}
	dfpair(stdout, "path", "%s", s.str().c_str());
	return cost;
}

void Segments::prinitial(FILE *out) const {
  dfrowhdr(out, "instance", 7, "radius", "initial x", "initial y", "initial rot", "goal x", "goal y", "goal, rot");
	for (auto sg = segs.begin(); sg != segs.end(); sg++) {
		dfrow(out, "instance", "guuuuuu", (double) sg->radius,
			(unsigned int) sg->start.x,
			(unsigned int) sg->start.y,
			(unsigned int) sg->start.rot,
			(unsigned int) sg->goal.x,
			(unsigned int) sg->goal.y,
			(unsigned int) sg->goal.rot);
	}
	dfpair(stdout, "width", "%u", width);
	dfpair(stdout, "height", "%u", height);
	dfpair(stdout, "number of angles", "%u", nangles);
}

std::vector<Segments::Oper> scanops(const std::string &str) {
	std::vector<Segments::Oper> ops;

	std::stringstream in(str);
	while (!in.eof()) {
		char tag;
		int seg;
		in >> tag >> seg;

		if (in.eof())
			break;

		switch (tag) {
		case 'm':
			int dx, dy;
			in >> dx >> dy;
			ops.push_back(Segments::Oper(Segments::Oper::Move,
				seg, dx, dy));
			break;

		case 'r':
			int delta;
			in >> delta;
			ops.push_back(Segments::Oper(Segments::Oper::Rotate,
				seg, delta, 0));
			break;

		default:
			fatal("Invalid operator tag: %c", tag);
		}
	}

	return ops;
}

static void dfline(std::vector<std::string> &line, void *aux) {
	Solution *sol = static_cast<Solution*>(aux);

	if (line.size() == 3 && line[0] == "#pair" && line[1] == "path") {
		if (sol->ops.size() > 0)
			warn("Multiple path keys in data file");
		sol->ops = scanops(line[2]);

	} else if (line.size() == 3 && line[0] == "#pair" && line[1] == "width") {
		sol->width = strtol(line[2].c_str(), NULL, 10);

	} else if (line.size() == 3 && line[0] == "#pair" && line[1] == "height") {
		sol->height = strtol(line[2].c_str(), NULL, 10);

	} else if (line.size() == 3 && line[0] == "#pair" && line[1] == "number of angles") {
		sol->nangles = strtol(line[2].c_str(), NULL, 10);

	} else if (line.size() == 9 && line[0] == "#altrow" && line[1] == "instance") {
		Segments::Seg s;
		s.radius = strtod(line[2].c_str(), NULL);
		s.start.x = strtol(line[3].c_str(), NULL, 10);
		s.start.y = strtol(line[4].c_str(), NULL, 10);
		s.start.rot = strtol(line[5].c_str(), NULL, 10);
		s.goal.x = strtol(line[6].c_str(), NULL, 10);
		s.goal.y = strtol(line[7].c_str(), NULL, 10);
		s.goal.rot = strtol(line[8].c_str(), NULL, 10);
		sol->segs.push_back(s);
	}
}

Solution readdf(FILE *in, FILE *echo) {
	Solution sol;
	dfread(in, dfline, &sol, echo);
	return sol;
}

// wrapind returns an index into an n element array
// that wraps around.
static int wrapind(int i, int n) {
	i %= n;
	if (i < 0)
		return i + n;
	return i;
}
