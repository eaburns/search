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

		Oper(OpType t = None, unsigned int x0 = 0, unsigned int y0 = 0) :
			type(t), x(x0), y(y0) { }

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

	class State {
	public:
		State() { }

		State(const DockRobot&, const std::vector<Loc>&&, int, unsigned int);

		State &operator = (State&o) {
			locs = o.locs;
			boxlocs = o.boxlocs;
			rbox = o.rbox;
			rloc = o.rloc;
			h = o.h;
			d = o.d;
			nleft = o.nleft;
			return *this;
		}

		State &operator = (State &&o) {
			std::swap(locs, o.locs);
			std::swap(boxlocs, o.boxlocs);
			rbox = o.rbox;
			rloc = o.rloc;
			h = o.h;
			d = o.d;
			nleft = o.nleft;
			return *this;
		}

		// locs is the state of each location.
		std::vector<Loc> locs;
		// boxlocs is the location of each box.
		std::vector<unsigned int> boxlocs;
		// rbox is the robot's contents (-1 is empty).
		int rbox;
		// rloc is the robot's location.
		unsigned int rloc;
		// h and d are the heuristic and distance estimates.
		Cost h, d;
		// nleft is the number of packages out of their goal location.
		unsigned int nleft;

		bool operator==(const State &o) const {
			if (rloc != o.rloc || rbox != o.rbox || nleft != o.nleft)
				return false;
			for (unsigned int i = 0; i < locs.size(); i++) {
				if (locs[i] != o.locs[i])
					return false;
			}				
			return true;
		}
	};

	class PackedState {
	public:
		PackedState() : sz(0), pos(0) { }

		~PackedState() {
			if (pos) delete[] pos;
		}

		bool operator==(const PackedState &o) const {
			for (unsigned int i = 0; i < sz; i++) {
				if (pos[i] != o.pos[i])
					return false;
			}
			return true;
		}

		unsigned int sz;
		unsigned int *pos;
	};

	State initialstate();

	unsigned long hash(const PackedState &p) const {
		return hashbytes(reinterpret_cast<unsigned char*>(p.pos),
			sizeof(p.pos[0])*p.sz);
	}

	Cost h(State &s) const {
		if (s.h < Cost(0)) {
			std::pair<float,float> p = hd(s);
			s.h = p.first;
			s.d = p.second;
		}
		return s.h;
	}

	Cost d(State &s) const {
		if (s.d < Cost(0)) {
			std::pair<float,float> p = hd(s);
			s.h = p.first;
			s.d = p.second;
		}
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

		Cost oldh, oldd;

		Edge(const DockRobot &d, State &s, const Oper &o) : state(s), dom(d) {
			apply(state, o);
			oldh = state.h;
			oldd = state.d;
			state.h = state.d = Cost(-1);
		}

		~Edge() {
			apply(state, revop);
			state.h = oldh;
			state.d = oldd;
		}

	private:

		void apply(State&, const Oper&);

		// needed to reverse an operator application.
		const DockRobot &dom;
	};

	void pack(PackedState&, const State&);

	State &unpack(State&, const PackedState&);

	void dumpstate(FILE*, const State&) const;

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

private:
	friend class State;
	friend bool compute_moves_test();
	friend bool compute_loads_test();

	// initheuristic initializes the heuristic values by computing
	// the shortest path between all locations.
	void initheuristic();

	std::pair<float, float> hd(const State&) const;

	// paths holds shortest path information between
	// locations used for computing the heuristic.
	std::vector< std::pair<float, float> > shortest;

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
