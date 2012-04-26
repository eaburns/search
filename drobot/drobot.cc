#include "drobot.hpp"
#include <limits>

unsigned int DockRobot::Loc::pop(unsigned int p) {
	assert (p < piles.size());

	Pile &pile = piles[p];
	unsigned int box = pile.stack.back();
	pile.stack.pop_back();

	if (pile.stack.empty()) {
		if (piles.size() > 1)
			piles[p] = piles.back();
		piles.pop_back();
		std::sort(piles.begin(), piles.end());
	}

	return box;
}

void DockRobot::Loc::push(unsigned int box, unsigned int p) {
	if (p >= piles.size()) {
		piles.push_back(Pile(box));
		std::sort(piles.begin(), piles.end());
	} else {
		piles[p].stack.push_back(box);
	}
}

unsigned int DockRobot::Loc::rmcrane(unsigned int c) {
	assert (c < cranes.size());
	unsigned int box = cranes[c];

	if (cranes.size() > 1)
		cranes[c] = cranes.back();
	cranes.pop_back();
	std::sort(cranes.begin(), cranes.end());

	return box;
}

void DockRobot::Loc::addcrane(unsigned int box) {
	cranes.push_back(box);
	std::sort(cranes.begin(), cranes.end());
}

const DockRobot::Oper DockRobot::Nop = { DockRobot::Oper::None, 0 };

DockRobot::DockRobot(unsigned int n) :
		nlocs(n),  nboxes(0), maxpiles(n, 0), maxcranes(n, 0), adj(n), initlocs(n) {
	for (unsigned int i = 0; i < n; i++) {
	for (unsigned int j = 0; j < n; j++)
		adj[i].push_back(std::numeric_limits<float>::infinity());
	}
}

DockRobot::DockRobot(FILE *in) {
	unsigned int nrobots, ncranes, npiles;

	if (fscanf(in, "nlocations: %u\n", &nlocs) != 1)
		fatal("Failed to read the number of locations");
	if (fscanf(in, "ncranes: %u\n", &ncranes) != 1)
		fatal("Failed to read the number of cranes");
	if (fscanf(in, "ncontainers: %u\n", &nboxes) != 1)
		fatal("Failed to read the number of boxes");
	if (fscanf(in, "npiles: %u\n", &npiles) != 1)
		fatal("Failed to read the number of piles");
	if (fscanf(in, "nrobots: %u\n", &nrobots) != 1)
		fatal("Failed to read the number of robots");
	if (nrobots != 1)
		fatal("Multiple robots are not supported");

	adj.resize(nlocs);
	initlocs.resize(nlocs);
	maxcranes.resize(nlocs);
	goal.resize(nboxes, -1);

	std::vector<unsigned int> pilelocs(npiles);

	for (unsigned int i = 0; i < nlocs; i++) {
		unsigned int loc;

		if (fscanf(in, "location %u\n", &loc) != 1)
			fatal("Failed to read the %uth location number", i);

		readadj(in, loc);
		readcranes(in, loc);
		readpiles(in, loc, pilelocs);
	}

	for (unsigned int i = 0; i < npiles; i++)
		readpile(in, i, pilelocs);

	for (unsigned int i = 0; i < nlocs; i++)
		maxpiles.push_back(initlocs[i].piles.size());

	for (unsigned int i = 0; i < nboxes; i++) {
		unsigned int box, loc;
		if (fscanf(in, "container %u at %u\n", &box, &loc) != 2)
			fatal("Failed to read the goal location for the %uth box", i);
		goal[box] = loc;
	}
}

void DockRobot::readadj(FILE *in, unsigned int loc) {
	boost::optional<std::string> line = readline(in);
	if (!line)
		fatal("Failed to read the adjacency list for location %u", loc);
	std::vector<std::string> ts = tokens(*line);
	if (ts.size() == 0 || ts[0] != "adjacent:")
		fatal("Failed to read the adjacency list for location %u", loc);
	if (ts.size() != nlocs + 1)
		fatal("Too few entries in the adjacency list for location %u", loc);
	for (unsigned int i = 1; i < ts.size(); i++)
		adj[loc].push_back(strtof(ts[i].c_str(), NULL));
}

void DockRobot::readcranes(FILE *in, unsigned int loc) {
	boost::optional<std::string> line = readline(in);
	if (!line)
		fatal("Failed to read the crane list for location %u", loc);
	std::vector<std::string> ts = tokens(*line);
	if (ts.size() == 0 || ts[0] != "cranes:")
		fatal("Failed to read the crane list for location %u", loc);
	maxcranes[loc] = ts.size() - 1;
}

void DockRobot::readpiles(FILE *in, unsigned int loc, std::vector<unsigned int> &pilelocs) {
	boost::optional<std::string> line = readline(in);
	if (!line)
		fatal("Failed to read the piles list for location %u", loc);
	std::vector<std::string> ts = tokens(*line);
	if (ts.size() == 0 || ts[0] != "piles:")
		fatal("Failed to read the piles list for location %u", loc);
	for (unsigned int i = 1; i < ts.size(); i++) {
		long pile = strtol(ts[i].c_str(), NULL, 10);
		pilelocs[pile] = loc;
	}
}

void DockRobot::readpile(FILE *in, unsigned int n, const std::vector<unsigned int> &pilelocs) {
	unsigned int p;

	if (fscanf(in, " pile %u", &p) != 1)
		fatal("Failed to read the %uth pile", n);
	if (fgetc(in) != '\n')
		fatal("Failed to read the new line on the %uth pile", n);

	boost::optional<std::string> line = readline(in);
 	std::vector<std::string> ts;
	if (line)
		ts = tokens(*line);

	Pile pile;
	for (unsigned int i = 0; i < ts.size(); i++) {
		long box = strtol(ts[i].c_str(), NULL, 10);
		pile.stack.push_back(box);
	}

	unsigned int loc = pilelocs[p];
	initlocs[loc].piles.push_back(pile);
}

DockRobot::Edge::Edge(DockRobot &d, State &s, Oper o) : state(s), dom(d) {
	s.h = s.d = 0;

	switch (o.type) {
	case Oper::Push: {
		unsigned int c = o.x;
		int p = o.y;
		Loc &l = s.locs[s.rloc];
		unsigned int box = l.rmcrane(c);
		unsigned int bottom = box;
		unsigned int sz = 0;
		if ((unsigned int) p < l.piles.size()) {
			bottom = l.piles[p].stack[0];
			sz = l.piles[p].stack.size();
		}
		l.push(box, p);

		p = l.findpile(bottom);
		assert (p >= 0);
		revop = Oper(Oper::Pop, p);
		cost = 0.05 * (sz + 1);	// from the OCaml code

		break;
	}
	case Oper::Pop: {
		int p = o.x;
		Loc &l = s.locs[s.rloc];
		assert ((unsigned int) p < l.piles.size());
		unsigned int sz = l.piles[p].stack.size();
		unsigned int bottom = l.piles[p].stack[0];
		unsigned int box = l.pop(p);
		l.addcrane(box);

		int c = l.findcrane(box);
		assert (c >= 0);
		if (sz == 1)
			p = l.piles.size();
		else
			p = l.findpile(bottom);
		revop = Oper(Oper::Push, c, p);
		cost = 0.05 * (sz + 1);	// from the OCaml code
		break;
	}
	case Oper::Load: {
		assert (s.rbox < 0);
		Loc &l = s.locs[s.rloc];
		unsigned int c = o.x;
		s.rbox = l.rmcrane(c);

		revop = Oper(Oper::Unload);
		cost = 0.01;	// from the OCaml code
		break;
	}
	case Oper::Unload: {
		assert (s.rbox >= 0);
		Loc &l = s.locs[s.rloc];
		unsigned int box = s.rbox;
		l.addcrane(box);
		assert (l.cranes.size() <= d.maxcranes[s.rloc]);	// not to many cranes, right?
		s.rbox = -1;

		int c = l.findcrane(box);
		assert (c >= 0);
		revop = Oper(Oper::Load, c);
		cost = 0.01;	// from the OCaml code
		break;
	}
	case Oper::Move: {
		unsigned int src = s.rloc, dst = o.x;

		// Are we moving a box out of its goal location
		// or to its goal location?
		if (s.rbox >= 0 && d.goal[s.rbox] >= 0) {
 			if ((unsigned int) d.goal[s.rbox] == s.rloc)
				s.nleft++;
			else if ((unsigned int) d.goal[s.rbox] == dst)
				s.nleft--;
		}

		s.rloc = dst;

		revop = Oper(Oper::Move, src);
		cost = d.adj[src][dst];
		break;
	}
	default:
		fatal("Unknown operator type %d", o.type);
	}
}

DockRobot::Edge::~Edge() {
	// creating the reverse edge undoes the operator.
	Edge e(dom, state, revop);
}

void DockRobot::computeops(State &s) const {
	const Loc &l = s.locs[s.rloc];

	// Push cranes onto piles.
	int lastpile = l.piles.size();
	if ((unsigned int) lastpile >= maxpiles[s.rloc])
		lastpile = maxpiles[s.rloc] - 1;
	for (unsigned int c = 0; c < l.cranes.size(); c++) {
		for (int p = 0; p <= lastpile; p++)
			s.ops.push_back(Oper(Oper::Push, c, p));
	}

	if (l.cranes.size() < maxcranes[s.rloc]) {	// empty cranes?

		// Pop piles onto a crane
		for (unsigned int p = 0; p < l.piles.size(); p++)
			s.ops.push_back(Oper(Oper::Pop, p));

		// Unload the robot
		if (s.rbox >= 0)
			s.ops.push_back(Oper(Oper::Unload));	
	}

	// Load the robot
	if (s.rbox < 0) {
		for (unsigned int c = 0; c < l.cranes.size(); c++)
			s.ops.push_back(Oper(Oper::Load, c));
	}

	// Move the robot
	for (unsigned int i = 0; i < nlocs; i++) {
		if (i == s.rloc)
			continue;
		s.ops.push_back(Oper(Oper::Move, i));
	}
}

// pos must be at least nboxes in size.
void DockRobot::boxlocs(const State &s, unsigned int pos[]) const {
	for (unsigned int i = 0; i < nboxes; i++)
		pos[i] = 0;

	unsigned int cur = 0;
	for (unsigned int i = 0; i < s.locs.size(); i++) {
		const Loc &l = s.locs[i];
		for (unsigned int c = 0; c < l.cranes.size(); c++)
			pos[l.cranes[c]] = cur + c;
		cur += maxcranes[i];
		for (unsigned int p = 0; p < l.piles.size(); p++) {
			for (unsigned int s = 0; s < l.piles[p].stack.size(); s++) {
				pos[l.piles[p].stack[s]] = cur + s + p*nboxes;
			}
		}
		cur += maxpiles[i]*nboxes;
	}
}
		