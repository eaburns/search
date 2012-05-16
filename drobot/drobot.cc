#include "drobot.hpp"
#include <limits>
#include <cmath>

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
	maxcranes.resize(nlocs, 0);
	maxpiles.resize(nlocs, 0);
	goal.resize(nboxes, -1);
	std::vector<unsigned int> pilelocs(npiles);

	long loc = -1;
	boost::optional<std::string> line = readline(in);
	while (line) {
		std::vector<std::string> toks = tokens(*line);
		if (toks.size() == 0)
			goto reloop;

		if (toks[0] == "location") {
			loc = strtol(toks[1].c_str(), NULL, 10);
			if (loc < 0 || loc >= nlocs)
				fatal("Invalid location %d", loc);

		} else if (toks[0] == "adjacent:") {
			if (toks.size() != nlocs + 1)
				fatal("Malformed adjacency list for location %ld", loc);

			for (unsigned int i = 1; i < toks.size(); i++)
				adj[loc].push_back(strtof(toks[i].c_str(), NULL));

		} else if (toks[0] == "cranes:") {
			maxcranes[loc] = toks.size()-1;

		} else if (toks[0] == "piles:") {
			for (unsigned int i = 1; i < toks.size(); i++) {
				long p = strtol(toks[i].c_str(), NULL, 10);
				if (p < 0 || p >= npiles)
					fatal("Malformed pile list, pile %ld is out of bounds", p);
				pilelocs[p] = loc;
				maxpiles[loc]++;
			}

		} else if (toks[0] == "pile") {
			if (toks.size() != 2)
				fatal("Malformed pile descriptor");

			long pnum = strtol(toks[1].c_str(), NULL, 10);
			if (pnum < 0 || pnum >= npiles)
				fatal("Malformed pile descriptor, pile %ld is out of bounds", pnum);

			line = readline(in);
			if (!line)
				goto reloop;
			toks = tokens(*line);
			Pile p;
			for (unsigned int i = 0; i < toks.size(); i++) {
				long box = strtol(toks[i].c_str(), NULL, 10);
				if (box < 0 || box >= nboxes)
					fatal("Malformed pile, box %ld is out of bounds", box);
				p.stack.push_back(box);
			}
			if (p.stack.size() > 0)
				initlocs[pilelocs[pnum]].piles.push_back(p);

		} else if (toks[0] == "container") {
			if (toks.size() != 4)
				fatal("Malformed goal descriptor");

			long box = strtol(toks[1].c_str(), NULL, 10);
			if (box < 0 || box >= nboxes)
				fatal("Out of bound container %ld in a goal descriptor", box);

			long dest = strtol(toks[3].c_str(), NULL, 10);
			if (dest < 0 || dest >= nlocs)
				fatal("Out of bound location %ld in goal descriptor", dest);

			goal[box] = dest;
		}
reloop:
		line = readline(in);
	}

	initheuristic();
}

DockRobot::State DockRobot::initialstate() {
	return State(*this, initlocs, -1, 0);
}

DockRobot::State::State(const DockRobot &dr, const std::vector<Loc> &ls,
	int rb, unsigned int rl) : locs(ls), rbox(rb), rloc(rl), h(-1), d(-1), nleft(0) {

	boxlocs.resize(dr.nboxes);

	for (unsigned int l = 0; l < locs.size(); l++) {
	for (unsigned int p = 0; p < locs[l].piles.size(); p++) {
	for (unsigned int s = 0; s < locs[l].piles[p].stack.size(); s++) {
		unsigned int box = locs[l].piles[p].stack[s];
		if ((unsigned int) dr.goal[box] != l)
			nleft++;
		boxlocs[box] = l;
	}
	}
	}
}

void DockRobot::Edge::apply(State &s, const Oper &o) {
	switch (o.type) {
	case Oper::Push: {
		unsigned int c = o.x;
		int p = o.y;
		assert (s.rloc < s.locs.size());
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
		assert (l.cranes.size() <= dom.maxcranes[s.rloc]);	// not to many cranes, right?
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
		if (s.rbox >= 0 && dom.goal[s.rbox] >= 0) {
 			if ((unsigned int) dom.goal[s.rbox] == s.rloc)
				s.nleft++;
			else if ((unsigned int) dom.goal[s.rbox] == dst)
				s.nleft--;
			s.boxlocs[s.rbox] = dst;
		}

		s.rloc = dst;

		revop = Oper(Oper::Move, src);
		cost = dom.adj[src][dst];
		break;
	}
	default:
		fatal("Unknown operator type %d", o.type);
	}

}

DockRobot::Operators::Operators(const DockRobot &d, const State &s) {
	const Loc &l = s.locs[s.rloc];

	// Push cranes onto piles.
	int lastpile = l.piles.size();
	if ((unsigned int) lastpile >= d.maxpiles[s.rloc])
		lastpile = d.maxpiles[s.rloc] - 1;
	for (unsigned int c = 0; c < l.cranes.size(); c++) {
		for (int p = 0; p <= lastpile; p++)
			ops.push_back(Oper(Oper::Push, c, p));
	}

	if (l.cranes.size() < d.maxcranes[s.rloc]) {	// empty cranes?

		// Pop piles onto a crane
		for (unsigned int p = 0; p < l.piles.size(); p++)
			ops.push_back(Oper(Oper::Pop, p));

		// Unload the robot
		if (s.rbox >= 0)
			ops.push_back(Oper(Oper::Unload));	
	}

	// Load the robot
	if (s.rbox < 0) {
		for (unsigned int c = 0; c < l.cranes.size(); c++)
			ops.push_back(Oper(Oper::Load, c));
	}

	// Move the robot
	for (unsigned int i = 0; i < d.nlocs; i++) {
		if (i == s.rloc)
			continue;
		ops.push_back(Oper(Oper::Move, i));
	}
}

DockRobot::Cost DockRobot::pathcost(const std::vector<State> &path, const std::vector<Oper> &ops) {
	State state = initialstate();
	Cost cost(0);
	for (int i = ops.size() - 1; i >= 0; i--) {
		State copy(state);
		Edge e(*this, copy, ops[i]);
		assert (e.state == path[i]);
		state = e.state;
		cost += e.cost;
	}
	assert (isgoal(state));
	return cost;
}

// pos must be at least nboxes in size.
void DockRobot::boxpos(const State &s, unsigned int pos[]) const {
	for (unsigned int i = 0; i < nboxes; i++)
		pos[i] = 0;

	unsigned int cur = 0;
	if (s.rbox >= 0)
		pos[s.rbox] = cur;
	cur++;

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

void DockRobot::initheuristic() {
	shortest.resize(nlocs*nlocs);
	for (unsigned int i = 0; i < nlocs; i++) {
	for (unsigned int j = 0; j < nlocs; j++) {
		float cost = adj[i][j];
		if (i == j)
			shortest[i*nlocs + j] = std::pair<float, float>(0, 0);
		else if (isinf(cost))
			shortest[i*nlocs + j] = std::pair<float, float>(cost, cost);
		else
			shortest[i*nlocs + j] = std::pair<float, float>(cost, 1);
	}
	}

	for (unsigned int k = 0; k < nlocs; k++) {
	for (unsigned int i = 0; i < nlocs; i++) {
	for (unsigned int j = 0; j < nlocs; j++) {
		float c = shortest[i*nlocs + k].first + shortest[k*nlocs + j].first;
		if (c < shortest[i*nlocs + j].first)
			shortest[i*nlocs + j].first = c;

		float d = shortest[i*nlocs + k].second + shortest[k*nlocs + j].second;
		if (c < shortest[i*nlocs + j].second)
			shortest[i*nlocs + j].second = d;
	}
	}
	}
}

std::pair<float, float> DockRobot::hd(const State &s) const {
	float h = 0, d = 0;

	for (unsigned int b = 0; b < goal.size(); b++) {
		if (goal[b] < 0)
			continue;
		const std::pair<float,float> &p = shortest[s.boxlocs[b]*nlocs + goal[b]];
		h += p.first;
		d += p.second;
	}

	return std::pair<float,float>(h, d);
}