#include "../utils/utils.hpp"
#include <cstdio>
#include <vector>
#include <algorithm>

struct DockRobot {

	struct Pile;
	struct Cranes;

	struct Loc {

		bool operator==(const Loc &o) const {
			return cranes == o.cranes && piles == o.piles;
		}

		bool operator!=(const Loc &o) const {
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
			unsigned int l = 0, u = piles.size();
			if (u == 0)
				return -1;

			// define: piles[-1] ≡ -∞ and piles[piles.size] ≡ ∞
			// invariant: l - 1 < box and u ≥ box
			while (l < u) {
				unsigned int m = ((u - l) >> 1) + l;
				if (piles[m].stack[0] < box)
					l = m + 1;	// l - 1 < box
				else
					u = m;	// u >= box
			}
			return (l < piles.size() && piles[l].stack[0] == box) ? l : -1;
		}

		// findcrane returns the index of the crane with the
		// given box.
		int findcrane(unsigned int box) {
			unsigned int l = 0, u = cranes.size();
			if (u == 0)
				return -1;

			while (l < u) {
				unsigned int m = ((u - l) >> 1) + l;
				if (cranes[m] < box)
					l = m + 1;
				else
					u = m;
			}
			return (l < cranes.size() && cranes[l] == box) ? l : -1;
		}

		std::vector<unsigned int> cranes;
		std::vector<Pile> piles;
	};

	struct Pile {

		Pile() { }

		// Construct a new pile with a single box.
		Pile(unsigned int b) : stack(1) {
			stack[0] = b;
		}

		// operator< orders piles by their bottom stack element.
		bool operator<(const Pile &o) const {
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

		bool operator==(const Oper &o) const {
			return x == o.x && type == o.type && y == o.y;
		}

		OpType type;
		unsigned int x, y;
	};

	static const Oper Nop;

	// creates a dummy instance with the given number of locations.
	DockRobot(unsigned int);

	DockRobot(FILE*);

	struct State {
		std::vector<Loc> locs;
		int rbox;	// the robot's contents (-1 is empty)
		unsigned int rloc;	// the robot's location

		Cost h, d;

		// nleft is the number of packages out of their goal location.
		unsigned int nleft;

		bool operator==(const State &o) const {
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

	unsigned long hash(const PackedState &p) const {
		unsigned int pos[nboxes+1];
		boxlocs(p, pos);
		pos[nboxes] = p.rloc;

		return hashbytes(reinterpret_cast<unsigned char*>(&pos[0]),
			sizeof(pos[0])*(nboxes+1));
	}

	Cost h(const State &s) const {
		return s.h;
	}

	Cost d(const State &s) const {
		return s.d;
	}

	// Is the given state a goal state?
	bool isgoal(const State &s) const {
		return s.nleft == 0;
	}

	struct Operators {
		Operators(const DockRobot&, const State&);

		unsigned int size() const {
			return ops.size();
		}

		Oper operator[](unsigned int i) const {
			return ops[i];
		}

	private:
		std::vector<Oper> ops;
	};

	struct Edge {
		Cost cost;
		Oper revop;
		State &state;

		Edge(const DockRobot &d, State &s, const Oper &o) : state(s), dom(d) {
			apply(state, o);
		}

		~Edge() {
			apply(state, revop);
		}

	private:

		void apply(State&, const Oper&);

		// needed to reverse an operator application.
		const DockRobot &dom;
	};

	void pack(PackedState &dst, const State &src) {
		dst = src;
	}

	State &unpack(const State &buf, PackedState &pkd) {
		return pkd;
	}

	void dumpstate(FILE *out, State &s) {
		fatal("Unimplemented");
	}

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

private:
	friend bool compute_moves_test();
	friend bool compute_loads_test();

	void computeops(State&) const;
	void boxlocs(const State&, unsigned int[]) const;

	// The number of various things in the dockyard.
	unsigned int nlocs, nboxes;

	// maxpiles is the maximum number of piles at a given location.
	std::vector<unsigned int> maxpiles; 

	// ncranes is the number of cranes at each location.
	std::vector<unsigned int> maxcranes;

	// adj is the adjacency matrix for locations.
	std::vector< std::vector<float> > adj;

	// initlocs is the initial location configurations.
	std::vector<Loc> initlocs;

	// goal is a vector of the desired location for each box
	std::vector<int> goal;
};
