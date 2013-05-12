#include "gridnav.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <limits>
#include <cassert>

GridNav::GridNav(GridMap *m, unsigned int x0, unsigned int y0,
		unsigned int x1, unsigned int y1) : map(m) {
	start = map->index(x0+1, y0+1);
	finish = map->index(x1+1, y1+1);
	reverseops();
}

GridNav::State GridNav::initialstate() const {
	State s;
	s.loc = start;
	return s;
}

void GridNav::reverseops() {
	unsigned int nrev = 0;
	for (unsigned int i = 0; i < map->nmvs; i++) {
	for (unsigned int j = 0; j < map->nmvs; j++) {
		if (map->mvs[i].dx != -map->mvs[j].dx || map->mvs[i].dy != -map->mvs[j].dy)
			continue;
		rev[i] = j;
		nrev++;
		break;
	} 
	}
	assert (nrev == map->nmvs);
}

GridNav::Cost GridNav::pathcost(const std::vector<State>&, const std::vector<Oper> &ops) const {
	std::vector<unsigned int> controls;
	GridNav::State state = initialstate();
	GridNav::Cost cost(0);
	for (int i = ops.size() - 1; i >= 0; i--) {
		GridNav::State copy(state);
		GridNav::Edge e(*this, copy, ops[i]);
		state = e.state;
		cost += e.cost;
		controls.push_back(ops[i]);
	}

	dfpair(stdout, "controls", "%s", controlstr(controls).c_str());

	assert (isgoal(state));
	return cost;
}

std::string controlstr(const std::vector<unsigned int> &controls) {
        std::string bytes;
        for (unsigned int i = 0; i < controls.size(); i++)
                bytes.push_back(controls[i] & 0xFF);
        return base64enc(runlenenc(bytes));
}

std::vector<unsigned int> controlvec(const std::string &enc) {
        std::string bytes = runlendec(base64dec(enc));
	std::vector<unsigned int> v;
	for (unsigned int i = 0; i < bytes.size(); i++)
	        v.push_back(bytes[i]);
        return v;
}

