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

static const Test tests[] = {
	Test("loc pop test", loc_pop_test),
	Test("loc push test", loc_push_test),
	Test("loc rmcrane test", loc_rmcrane_test),
	Test("loc addcrane test", loc_addcrane_test),
	Test("loc crane equality test", loc_crane_equality_test),
	Test("loc pile equality test", loc_pile_equality_test),
	Test("loc full equality test", loc_full_equality_test),
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