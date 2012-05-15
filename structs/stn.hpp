#include <vector>
#include <limits>
#include <utility>
#include <cstdio>

class Stn {
public:
	typedef long Time;

	Stn(unsigned int);

	Stn(const Stn &);

	void grow(unsigned int);

	void undo(void);

	Time lower(unsigned int n) const { return nodes[n].tozero; }

	Time upper(unsigned int n) const { return nodes[n].fromzero; }

	unsigned int nnodes(void) const { return nodes.size(); }

	struct Constraint {
		unsigned int i, j;
		Time a, b;

		Constraint(void) { }

		Constraint(unsigned int i, unsigned int j, Time a, Time b) :
			i(i), j(j), a(a), b(b) { }
	};

	struct Before : Constraint {
		Before(unsigned int i, unsigned int j, Time a) :
			Constraint(i, j, a, Stn::inf()) { }
	};

	struct After : Constraint {
		After(unsigned int i, unsigned int j, Time a) :
			Constraint(i, j, a, Stn::inf()) { }
	};

	struct NoEarlier : Constraint {
		NoEarlier(unsigned int i, Time t) :
			Constraint(0, i, t, Stn::inf()) { }
	};

	struct NoLater : Constraint {
		NoLater(unsigned int i, Time t) :
			Constraint(0, i, Stn::neginf(), t) { }
	};

	struct InWindow : Constraint {
		InWindow(unsigned int i, Time s, Time e) :
			Constraint(0, i, s, e) { }
	};

	bool add(const Constraint &);

	bool eq(const Stn &) const;

	static Time inf(void) {
		if (std::numeric_limits<Time>::has_infinity)
			return std::numeric_limits<Time>::infinity();
		return std::numeric_limits<Time>::max();
	}

	static Time neginf(void) {
		if (std::numeric_limits<Time>::has_infinity)
			return -std::numeric_limits<Time>::infinity();
		return std::numeric_limits<Time>::min();
	}

	void output(FILE*) const;

private:

	static Time subclamp(Time a, Time b) {
		if (b >= 0) {
			if (a < Stn::neginf() + b)
				return Stn::neginf();
			return a - b;
		}
		if (a > Stn::inf() + b)	// b is negative
			return Stn::inf();
		return a - b;	
	}

	static Time addclamp(Time a, Time b) {
		if (b >= 0) {
			if (a > Stn::inf() - b)
				return Stn::inf();
			return a + b;
		}
		if (a < Stn::neginf() - b)	// b is positive
			return Stn::neginf();
		return a + b;
	}

	struct Node;
	typedef std::pair<Node*,Time> Arc;

	struct Node {
		std::vector<Arc> out;
		std::vector<Arc> in;

		unsigned int id;
		Time tozero;
		Time fromzero;

		Node(void) {
			tozero = Stn::neginf();
			fromzero = Stn::inf();
		}

		bool eq(const Node&) const;
		void output(FILE*) const;
	};

	struct Undo {
		std::vector<Node*> popout;
		std::vector< std::pair<Node*, Time> > prevto;
		std::vector< std::pair<Node*, Time> > prevfrom;

		bool eq(const Undo&) const;
		void output(FILE*) const;
	};

	bool propagate(Undo&, const Constraint&);
	bool proplower(Undo&, bool[], bool[], Node&);
	bool propupper(Undo&, bool[], bool[], Node&);
	void addarcs(Undo&, const Constraint&);

	std::vector<Node> nodes;
	std::vector<Undo> undos;
};
