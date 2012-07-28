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

const DockRobot::Oper DockRobot::Nop = DockRobot::Oper(DockRobot::Oper::None);

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
	State s(*this, std::vector<Loc>(initlocs), -1, 0);
	PackedState pkd;
	pack(pkd, s);
	State buf;
	State &s1 = unpack(buf, pkd);
	assert (s == s1);
	return s;
}

DockRobot::State::State(const DockRobot &dr, const std::vector<Loc> &&ls,
	int rb, unsigned int rl) : locs(ls), rbox(rb), rloc(rl), h(-1), d(-1), nleft(0) {

	boxlocs.resize(dr.nboxes, dr.nlocs+1);

	for (unsigned int l = 0; l < locs.size(); l++) {
		for (unsigned int c = 0; c < locs[l].cranes.size(); c++) {
			unsigned int box = locs[l].cranes[c];
			if (dr.goal[box] >= 0 && (unsigned int) dr.goal[box] != l)
				nleft++;
			boxlocs[box] = l;
		}
		for (unsigned int p = 0; p < locs[l].piles.size(); p++) {
		for (unsigned int s = 0; s < locs[l].piles[p].stack.size(); s++) {
			unsigned int box = locs[l].piles[p].stack[s];
			assert (box < dr.nboxes);
			if (dr.goal[box] >= 0 && (unsigned int) dr.goal[box] != l)
				nleft++;
			boxlocs[box] = l;
		}
		}
	}
	if (rbox >= 0) {
		if (dr.goal[rbox] >= 0 && (unsigned int) dr.goal[rbox] != rloc)
			nleft++;
		boxlocs[rbox] = rloc;
	}
}

DockRobot::State::State(const State &s) :
	locs(s.locs),
	boxlocs(s.boxlocs),
	rbox(s.rbox),
	rloc(s.rloc),
	h(s.h),
	d(s.d),
	nleft(s.nleft) {
}

// LoadCost is the cost of loading and unloading the robot.
const double LoadCost = 0.01;

// PopCostFact is the cost of poping from a stack, given as a
// factor of the stack's height.
const double PopCostFact = 0.05;

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
		cost = PopCostFact * (sz + 1);	// from the OCaml code

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
		cost = PopCostFact * (sz + 1);	// from the OCaml code
		break;
	}
	case Oper::Load: {
		assert (s.rbox < 0);
		Loc &l = s.locs[s.rloc];
		unsigned int c = o.x;
		s.rbox = l.rmcrane(c);

		revop = Oper(Oper::Unload);
		cost = LoadCost;
		break;
	}
	case Oper::Unload: {
		assert (s.rbox >= 0);
		Loc &l = s.locs[s.rloc];
		unsigned int box = s.rbox;
		l.addcrane(box);
		assert (l.cranes.size() <= dom.maxcranes[s.rloc]);
		s.rbox = -1;

		int c = l.findcrane(box);
		assert (c >= 0);
		revop = Oper(Oper::Load, c);
		cost = LoadCost;
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
		if (!(e.state == path[i])) {
			fprintf(stderr, "e.state:\n");
			dumpstate(stderr, e.state);
			fprintf(stderr, "\npath[%u]:\n", i);
			dumpstate(stderr, path[i]);
		}
		assert (e.state == path[i]);
		state = e.state;
		cost += e.cost;
	}
	assert (isgoal(state));
	return cost;
}

void DockRobot::pack(PackedState  &dst, const State &s) {
	unsigned int *pos = new unsigned int[nboxes+1];
	for (unsigned int i = 0; i < nboxes+1; i++)
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
	pos[nboxes] = s.rloc;
	dst.pos = pos;
	dst.sz = nboxes+1;
}

DockRobot::State &DockRobot::unpack(State &buf, const PackedState &pkd) {
	unsigned int rloc = pkd.pos[nboxes];
	int rbox = -1;
	std::vector<Loc> locs(nlocs);

	for (unsigned int b = 0; b < nboxes; b++) {
		unsigned int p = pkd.pos[b];
		if (p == 0) {
			rbox = b;
			goto nextbox;
		}
		p--;

		for (unsigned int l = 0; l < nlocs; l++) {
			Loc &loc = locs[l];

			if (p < maxcranes[l]) {
				if (loc.cranes.size() <= p)
					loc.cranes.resize(p+1, nboxes+1);
				loc.cranes[p] = b;
				goto nextbox;
			}
			p -= maxcranes[l];

			if (p < maxpiles[l]*nboxes) {
				unsigned int pid = p/nboxes;
				unsigned int ht = p%nboxes;

				if (loc.piles.size() <= pid)
					loc.piles.resize(pid+1);
				Pile &pile = loc.piles[pid];
				if (pile.stack.size() <= ht)
					pile.stack.resize(ht+1, nboxes+1);
				pile.stack[ht] = b;
				goto nextbox;
			}
			p -= maxpiles[l]*nboxes;
		}
		fatal("unreachable");
nextbox:;
	}

	buf = State(*this, std::move(locs), rbox, rloc);
	return buf;
}

void DockRobot::dumpstate(FILE *out, const State &s) const {
	fprintf(out, "h: %g\n", s.h);
	fprintf(out, "d: %g\n", s.d);
	fprintf(out, "robot location: %u\n", s.rloc);
	fprintf(out, "robot contents: %d\n", s.rbox);
	fprintf(out, "out of place boxes: %d\n", s.nleft);
	for (unsigned int l = 0; l < s.locs.size(); l++) {
		fprintf(out, "loc %u:\n", l);
		const Loc &loc = s.locs[l];
		for (unsigned int c = 0; c < loc.cranes.size(); c++)
			fprintf(out, "	crane: %u\n", loc.cranes[c]);
		for (unsigned int p = 0; p < loc.piles.size(); p++) {
			const Pile &pile = loc.piles[p];
			fprintf(out, "	pile:");
			for (unsigned int h = 0; h < pile.stack.size(); h++)
				fprintf(out, " %u", pile.stack[h]);
			fprintf(out, "\n");
		}
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
	if (s.nleft == 0)
		return std::pair<float,float>(0, 0);

	float h = 0, d = 0;

	// Each out-of-place box must move to its goal.
	for (unsigned int b = 0; b < goal.size(); b++) {
		if (goal[b] < 0)
			continue;
		const std::pair<float,float> &p = shortest[s.boxlocs[b]*nlocs + goal[b]];
		h += p.first;
		d += p.second;
	}

	// All execpt for the last out-of-place box must be unloaded
	// from the robot.
	h += (s.nleft-1)*LoadCost;
	d += s.nleft-1;

	for (unsigned int l = 0; l < s.locs.size(); l++) {
		const Loc &loc = s.locs[l];

		// Each out-of-place box on a crane must be
		// loaded onto the robot.
		for (unsigned int c = 0; c < loc.cranes.size(); c++)  {
			unsigned int box = loc.cranes[c];
			if (goal[box] < 0 || (unsigned int) goal[box] == l)
				continue;
			h += LoadCost;
			d += 1;
		}

		// Each out-of-place box on a stack must be popped
		// onto a crane, then moved to the robot.
		for (unsigned int p = 0; p < loc.piles.size(); p++) {
			const Pile &pile = loc.piles[p];
			for (unsigned int ht = 0; ht < pile.stack.size(); ht++) {
				unsigned int box = pile.stack[ht];
				if (goal[box] < 0 || (unsigned int) goal[box] == l)
					continue;
				h += PopCostFact*(ht + 2) + LoadCost;
				d += 2;
			}
		}
	}

	return std::pair<float,float>(h, d);
}