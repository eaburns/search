// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#pragma once

#include "../gridnav/gridnav.hpp"
#include "unknowngridmap.hpp"

struct UnknownGridNav : public GridNav {
	UnknownGridNav(GridMap* gm, unsigned int sx, unsigned int sy,
		unsigned int gx, unsigned int gy) : GridNav(gm, sx, sy, gx, gy) {}

	void act(const State &state, const Oper &op) {
                ((UnknownGridMap*)map)->revealCells(state.loc);
	}
};
