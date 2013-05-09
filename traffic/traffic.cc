#include "traffic.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <limits>
#include <cassert>

Traffic::Traffic(GridMap *m, unsigned int x0, unsigned int y0,
		 unsigned int x1, unsigned int y1) : map(m) {
	start = map->index(x0, y0);
	finish = map->index(x1, y1);
}

Traffic::State Traffic::initialstate() const {
	State s;
	s.loc = start;
	s.t = 0;
	return s;
}

Traffic::Cost Traffic::pathcost(const std::vector<State>&, const std::vector<Oper> &ops) const {
	std::vector<unsigned int> controls;
	Traffic::State state = initialstate();
	Traffic::Cost cost(0);

	int t = 0;
	for (int i = ops.size() - 1; i >= 0; i--) {

//		std::pair<int,int> loc = map->coord(state.loc);
//		for(unsigned int j = 0; j < map->obstacles.size(); j++) {
//			std::pair<int,int> obs = map->obstacles[j].positionAt(map->w, map->h, t);
//			assert(loc != obs);
//		}
		t++;
		Traffic::State copy(state);
		Traffic::Edge e(*this, copy, ops[i]);
		controls.push_back(ops[i]);
		state = e.state;
		cost += e.cost;
	}

	assert (isgoal(state));
	dfpair(stdout, "controls", "%s", controlstr(controls).c_str());
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
