#include "../utils/utils.hpp"
#include "drobot.hpp"
#include <cstdlib>
#include <ctime>

bool loc_pop_test() {
	bool ok = true;

	DockRobot::Loc loc;
	loc.piles.push_back(DockRobot::Pile(0));
	loc.piles.push_back(DockRobot::Pile(1));
	loc.piles.push_back(DockRobot::Pile(2));

	unsigned int box = loc.pop(1);
	if (box != 1) {
		testpr("Expected to pop box 1 but got %u", box);
		ok = false;
	}

	if (loc.piles.size() != 2) {
		testpr("Expected 2 piles after popping but got %u", loc.piles.size());
		ok = false;
	}

	if (loc.piles[0].stack[0] != 0) {
		testpr("Expected the 0th piles after popping to be 0 but got %u", loc.piles[0].stack[0]);
		ok = false;
	}

	if (loc.piles[1].stack[0] != 2) {
		testpr("Expected the 1st piles after popping to be 2 but got %u", loc.piles[1].stack[0]);
		ok = false;
	}

	return ok;
}

bool loc_push_test() {
	bool ok = true;

	DockRobot::Loc loc;
	loc.piles.push_back(DockRobot::Pile(0));
	loc.piles.push_back(DockRobot::Pile(3));
	loc.piles.push_back(DockRobot::Pile(6));

	loc.push(5, 1);
	if (loc.piles[1].stack.size() != 2) {
		testpr("Expected stack 1 to contain 2 element but got %u", loc.piles[1].stack.size());
		ok = false;
	}
	if (loc.piles[1].stack.back() != 5) {
		testpr("Expected stack 1 to have its top be element 5 but got %u", loc.piles[1].stack.back());
		ok = false;
	}

	loc.push(5, 3);
	if (loc.piles.size() != 4) {
		testpr("Expected 4 piles after pushing off the end but got %u", loc.piles.size());
		ok = false;
	}
	if (loc.piles[0].stack[0] != 0) {
		testpr("Expected plie 0 to have box 0 but it has box %u", loc.piles[0].stack[0]);
		ok = false;
	}
	if (loc.piles[1].stack[0] != 3) {
		testpr("Expected plie 1 to have box 3 but it has box %u", loc.piles[1].stack[0]);
		ok = false;
	}
	if (loc.piles[2].stack[0] != 5) {
		testpr("Expected plie 2 to have box 5 but it has box %u", loc.piles[2].stack[0]);
		ok = false;
	}
	if (loc.piles[3].stack[0] != 6) {
		testpr("Expected plie 3 to have box 6 but it has box %u", loc.piles[3].stack[0]);
		ok = false;
	}

	return ok;
}

bool loc_rmcrane_test() {
	bool ok = true;

	DockRobot::Loc loc;
	loc.cranes.push_back(1);
	loc.cranes.push_back(2);
	loc.cranes.push_back(5);

	loc.rmcrane(1);
	if (loc.cranes.size() != 2) {
		testpr("Expected 2 cranes but got %u", loc.cranes.size());
		ok = false;
	}
	if (loc.cranes[0] != 1) {
		testpr("Expected crane 0 to have box 1 but got %u", loc.cranes[0]);
		ok = false;
	}
	if (loc.cranes[1] != 5) {
		testpr("Expected crane 1 to have box 5 but got %u", loc.cranes[1]);
		ok = false;
	}

	return ok;
}

bool loc_addcrane_test() {
	bool ok = true;

	DockRobot::Loc loc;
	loc.cranes.push_back(1);
	loc.cranes.push_back(2);
	loc.cranes.push_back(5);

	loc.addcrane(4);
	if (loc.cranes.size() != 4) {
		testpr("Expected 4 cranes but got %u", loc.cranes.size());
		ok = false;
	}
	if (loc.cranes[0] != 1) {
		testpr("Expected crane 0 to have box 1 but got %u", loc.cranes[0]);
		ok = false;
	}
	if (loc.cranes[1] != 2) {
		testpr("Expected crane 1 to have box 2 but got %u", loc.cranes[1]);
		ok = false;
	}
	if (loc.cranes[2] != 4) {
		testpr("Expected crane 2 to have box 4 but got %u", loc.cranes[2]);
		ok = false;
	}
	if (loc.cranes[3] != 5) {
		testpr("Expected crane 3 to have box 5 but got %u", loc.cranes[3]);
		ok = false;
	}

	return ok;
}

bool loc_crane_equality_test() {
	bool ok = true;
	DockRobot::Loc a, b;

	if (a != b) {
		testpr("Expected two empty locations to be equal");
		ok = false;
	}

	a.addcrane(1);
	if (a == b) {
		testpr("Expected locations with different number of cranes to be unequal");
		ok = false;
	}

	b.addcrane(0);
	if (a == b) {
		testpr("Expected locations with different cranes to be unequal");
		ok = false;
	}

	a.addcrane(0);
	b.addcrane(1);
	if (a != b) {
		testpr("Expected locations with matching cranes to be equal");
		ok = false;
	}

	return ok;
}

bool loc_pile_equality_test() {
	bool ok = true;
	DockRobot::Loc a, b;

	if (a != b) {
		testpr("Expected two empty locations to be equal");
		ok = false;
	}

	a.push(1, 0);
	if (a == b) {
		testpr("Expected locations with different number of piles to be unequal");
		ok = false;
	}

	b.push(0, 0);
	if (a == b) {
		testpr("Expected locations with different piles to be unequal");
		ok = false;
	}

	a.push(0, 1);
	b.push(1, 1);
	if (a != b) {
		testpr("Expected locations with matching piles to be equal");
		ok = false;
	}

	return ok;
}

bool loc_full_equality_test() {
	bool ok = true;
	DockRobot::Loc a, b;

	if (a != b) {
		testpr("Expected two empty locations to be equal");
		ok = false;
	}

	a.addcrane(1);
	b.addcrane(0);
	a.push(1, 0);
	b.push(0, 0);
	if (a == b) {
		testpr("Expected locations with different cranes and piles to be unequal");
		ok = false;
	}

	a.addcrane(0);
	b.addcrane(1);
	if (a == b) {
		testpr("Expected locations with matching cranes but different piles to be unequal");
		ok = false;
	}

	a.push(0, 1);
	b.push(1, 1);
	if (a != b) {
		testpr("Expected locations with matching cranes and piles to be equal");
		ok = false;
	}

	a.addcrane(3);
	b.addcrane(4);
	if (a == b) {
		testpr("Expected locations with matching piles but different cranes to be unequal");
		ok = false;
	}

	return ok;
}

bool compute_moves_test() {
	bool ok = true;

	DockRobot dr(3);
	DockRobot::State s;
	s.locs.resize(3);
	s.rloc = 0;

	unsigned int n = dr.nops(s);
	if (n != 2) {
		testpr("Expected 2 move operations but got %u\n", n);
		ok = false;
	}

	std::vector<bool> seen(3, false);
	for (unsigned int i = 0; i < n; i++) {
		unsigned int dst = dr.nthop(s, i).x;
		if (dst > seen.size()) {
			testpr("Move to out of bound location %u\n", dst);
			ok = false;
		} else {
			seen[dst] = true;
		}
	}

	if (seen[0]) {
		testpr("Move to the current location\n");
		ok = false;
	}
	if (!seen[1]) {
		testpr("No move to location 1\n");
		ok = false;
	}
	if (!seen[2]) {
		testpr("No move to location 2\n");
		ok = false;
	}

	return ok;
}

bool compute_loads_test() {
	bool ok = true;

	DockRobot dr(1);
	dr.maxcranes[0] = 2;
	dr.maxpiles[1] = 0;
	DockRobot::State s;
	s.locs.resize(1);
	s.rloc = 0;
	s.rbox = -1;
	s.locs[0].cranes.push_back(0);
	s.locs[0].cranes.push_back(1);

	unsigned int n = dr.nops(s);
	if (n != 2) {
		testpr("Expected 2 operators but got %u\n", n);
		ok = false;
	}

	std::vector<bool> seen(2, false);
	for (unsigned int i = 0; i < n; i++) {
		unsigned int cr = dr.nthop(s, i).x;
		if (cr > seen.size()) {
			testpr("Load from out of bound crane %u", cr);
			ok = false;
		} else {
			seen[cr] = true;
		}
	}
	if (!seen[0]) {
		testpr("No load from crane 0");
		ok = false;
	}
	if (!seen[1]) {
		testpr("No load from crane 1");
		ok = false;
	}

	// what if the robot isn't empty?
	s.rbox = 0;
	s.hasops = false;
	s.ops.clear();
	n = dr.nops(s);
	if (n != 0) {
		testpr("Loaded into a full robot");
		ok = false;
	}

	return ok;
}

bool find_pile_test() {
	bool ok = true;

	DockRobot::Loc l;

	int ind = l.findpile(0);
	if (ind >= 0) {
		testpr("Find succeeded in an empty location at index %d\n", ind);
		ok = false;
	}

	l.piles.push_back(DockRobot::Pile(0));
	l.piles.push_back(DockRobot::Pile(1));
	l.piles.push_back(DockRobot::Pile(2));

	for (int i = 0; i < 3; i++) {
		ind = l.findpile(i);
		if (ind != i) {
			testpr("Expected box %d at index %d, got index %d\n", i, i, ind);
			ok = false;
		}
	}

	ind = l.findpile(3);
	if (ind >= 0) {
		testpr("Find succeeded for a box that is not at the location at index %d\n", ind);
		ok = false;
	}

	// test an even number of piles
	l.piles.push_back(DockRobot::Pile(3));
	for (int i = 0; i < 4; i++) {
		ind = l.findpile(i);
		if (ind != i) {
			testpr("Expected box %d at index %d, got index %d\n", i, i, ind);
			ok = false;
		}
	}

	ind = l.findpile(4);
	if (ind >= 0) {
		testpr("Find succeeded for a box that is not at the location at index %d\n", ind);
		ok = false;
	}

	return ok;
}

bool find_crane_test() {
	bool ok = true;

	DockRobot::Loc l;

	int ind = l.findpile(0);
	if (ind >= 0) {
		testpr("Find succeeded in an empty location at index %d\n", ind);
		ok = false;
	}

	l.addcrane(0);
	l.addcrane(1);
	l.addcrane(2);

	for (int i = 0; i < 3; i++) {
		ind = l.findcrane(i);
		if (ind != i) {
			testpr("Expected box %d at index %d, got index %d\n", i, i, ind);
			ok = false;
		}
	}

	ind = l.findcrane(3);
	if (ind >= 0) {
		testpr("Find succeeded for a box that is not at the location at index %d\n", ind);
		ok = false;
	}

	// test an even number of cranes
	l.addcrane(3);
	for (int i = 0; i < 4; i++) {
		ind = l.findcrane(i);
		if (ind != i) {
			testpr("Expected box %d at index %d, got index %d\n", i, i, ind);
			ok = false;
		}
	}

	ind = l.findcrane(4);
	if (ind >= 0) {
		testpr("Find succeeded for a box that is not at the location at index %d\n", ind);
		ok = false;
	}

	return ok;
}

static const Test tests[] = {
	Test("loc pop test", loc_pop_test),
	Test("loc push test", loc_push_test),
	Test("loc rmcrane test", loc_rmcrane_test),
	Test("loc addcrane test", loc_addcrane_test),
	Test("loc crane equality test", loc_crane_equality_test),
	Test("loc pile equality test", loc_pile_equality_test),
	Test("loc full equality test", loc_full_equality_test),
	Test("compute moves test", compute_moves_test),
	Test("compute loads test", compute_loads_test),
	Test("find pile test", find_pile_test),
	Test("find crane test", find_crane_test),
};
enum { Ntests = sizeof(tests) / sizeof(tests[0]) };

static const Benchmark benches[] = { };
enum { Nbenches = sizeof(benches) / sizeof(benches[0]) };

int main(int argc, const char *argv[]) {
	const char *regexp = ".*";
	if (argc == 2)
		regexp = argv[1];

	srand(time(NULL));

	bool ok = runtests(tests, Ntests, regexp);
	runbenches(benches, Nbenches, regexp);

	return ok ? 0 : 1;
}