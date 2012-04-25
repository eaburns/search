#include "../utils/utils.hpp"
#include <cstdio>
#include <vector>
#include <algorithm>

struct DockRobot {

	struct Pile;
	struct Cranes;

	struct Loc {

		bool operator==(Loc &o) {
			return cranes == o.cranes && piles == o.piles;
		}

		bool operator!=(Loc &o) {
			return !(*this == o);
		}

		// pop pops the given pile, returning the box.
		unsigned int pop(unsigned int p);

		// push pushes the given box into the given pile
		// If the pile number is greater than the current
		// number of piles then a new pile is created.
		void push(unsigned int b, unsigned int p);

		// rmcrane removes the crane, returning its box.
		unsigned int rmcrane(unsigned int);

		// addcrane adds a new crane with the given box.
		void addcrane(unsigned int);

		// findpile returns the index of the pile with the
		// given bottom box.
		int findpile(unsigned int box) {
			for (unsigned int i = 0; i < piles.size(); i++) {
				if (piles[i].stack[0] == box)
					return i;
			}
			return -1;
		}

		// findcrane returns the index of the crane with the
		// given box.
		int findcrane(unsigned int box) {
			for (unsigned int i = 0; i < cranes.size(); i++) {
				if (cranes[i] == box)
					return i;
			}
			return -1;
		}

		std::vector<unsigned int> cranes;
		std::vector<Pile> piles;

	private:

		// sorted is true if the cranes and piles are
		// already sorted.
		bool sorted;
	};

	struct Pile {

		Pile() { }

		// Construct a new pile with a single box.
		Pile(unsigned int b) {
			stack.push_back(b);
		}

		// operator< orders piles by their bottom stack element.
		bool operator<(const Pile &o) const {
			assert (!stack.empty());
			assert (!o.stack.empty());
			return stack[0] < o.stack[0];
		}

		bool operator==(const Pile &o) const {
			return stack == o.stack;
		}

		// stack is the stack of boxes in this pile.
		std::vector<unsigned int> stack;
	};

	typedef float Cost;

	struct Oper {
		enum OpType { None, Push, Pop, Load, Unload, Move };

		Oper(OpType t = None, unsigned int _x = 0, unsigned int _y = 0) :
			type(t), x(_x), y(_y) { }

		OpType type;
		unsigned int x, y;
	};

	static const Oper Nop;

	DockRobot(FILE*);

	struct State {

		std::vector<Loc> locs;
		int rbox;	// the robot's contents (-1 is empty)
		unsigned int rloc;	// the robot's location

		// nleft is the number of packages out of their goal location.
		unsigned int nleft;

		Cost h;
		unsigned int d;

		bool hasops;
		std::vector<Oper> ops;

		unsigned long hash(void) const {
			return -1;
		}

		bool eq(State &o) {
			if (rloc != o.rloc)
				return false;
			for (unsigned int i = 0; i < locs.size(); i++) {
				if (locs[i] != o.locs[i])
					return false;
			}
			return true;
		}
	};

	typedef State PackedState;

	State initialstate(void);

	Cost h(State &s) {
		return s.h;
	}

	Cost d(State &s) {
		return s.d;
	}

	// Is the given state a goal state?
	bool isgoal(State &s) {
		return s.nleft == 0;
	}

	unsigned int nops(State &s) {
		if (!s.hasops)
			computeops(s);
		return s.ops.size();
	}

	Oper nthop(State &s, unsigned int n) {
		return s.ops[n];
	}

	struct Edge {
		Cost cost;
		Oper revop;
		State &state;

		Edge(DockRobot&, State&, Oper);
		~Edge();

	private:

		// needed to reverse an operator application.
		DockRobot &dom;
	};

	void pack(PackedState &dst, State &src) {
		dst = src;
	}

	State &unpack(State &buf, PackedState &pkd) {
		return pkd;
	}

	void dumpstate(FILE *out, State &s) {
		fatal("Unimplemented");
	}

private:

	void readadj(FILE*, unsigned int);
	void readcranes(FILE*, unsigned int);
	void readpiles(FILE*, unsigned int, std::vector<unsigned int>&);
	void readpile(FILE*, unsigned int, const std::vector<unsigned int>&);
	void computeops(State&) const;

	// The number of various things in the dockyard.
	unsigned int nlocs, ncranes, nboxes, npiles;

	// maxpiles is the maximum number of piles at a given location.
	std::vector<unsigned int> maxpiles; 

	// ncranes is the number of cranes at each location.
	std::vector<unsigned int> maxcranes;

	// adj is the adjacency matrix for locations.
	std::vector< std::vector<float> > adj;

	// initlocs is the initial location configurations.
	std::vector<Loc> initlocs;

	// goal is a vector of the desired location for each box
	std::vector<unsigned int> goal;
};
