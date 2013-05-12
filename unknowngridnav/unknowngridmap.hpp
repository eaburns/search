#pragma once

#include "../gridnav/gridmap.hpp"

struct UnknownGridMap : public GridMap {
	UnknownGridMap(FILE* in) : GridMap(in) {
		unknown.resize(sz, true);
	}

	virtual bool ok(int loc, const Move &m) const {
		std::pair<int,int> c = coord(loc);
		if(c.first <= 0 || c.first >= (int)w || 
			c.second <= 0 || c.second >= (int)h)
			return false;

		return unknown[loc]  || GridMap::ok(loc, m);
	}

        void revealCells(unsigned int loc) {
		for(int dx = -1; dx <= 1; dx++)
			for(int dy = -1; dy <= 1; dy++)
				unknown[loc + dx + w * dy] = false;
        }

	std::vector<bool> unknown;
};
