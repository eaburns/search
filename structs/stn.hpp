#include <vector>
#include <limits>
#include <utility>

class Stn {
public:
	typedef int Time;

	Stn(unsigned int);

	Stn(const Stn &);

	void add(unsigned int);

	unsigned int nnodes(void) { return nodes.size(); }

	struct Constraint {
		unsigned int i, j;
		Time a, b;
	};

	bool add(const Constraint &);

	void undo(void);

	Time lower(unsigned int n) { return nodes[n].tozero; }

	Time upper(unsigned int n) { return nodes[n].fromzero; }

private:
	static Time inf(void) {
		return std::numeric_limits<Time>::max();
	}

	static Time neginf(void) {
		return std::numeric_limits<Time>::min();
	}

	static Time sub_clamp(Time a, Time b) {
		if (b >= 0) {
			Time t = Stn::neginf() + b;
			if (a < t)
				return Stn::neginf();
			return a - b;
		}
		Time t = Stn::inf() + b;
		if (a > t)
			return Stn::inf();
		return a - b;	
	}

	static Time add_clamp(Time a, Time b) {
		if (b >= 0) {
			Time t = Stn::inf() - b;
			if (a > t)
				return Stn::inf();
			return a + b;
		}
		Time t = Stn::neginf() - b;
		if (a < t)
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
	};

	struct Undo {
		std::vector<Node*> popout;
		std::vector< std::pair<Node*, Time> > prevto;
		std::vector< std::pair<Node*, Time> > prevfrom;
	};

	bool propagate(Undo&, const Constraint&);
	bool proplower(Undo&, bool[], bool[], Node&);
	bool propupper(Undo&, bool[], bool[], Node&);
	void addarcs(Undo&, const Constraint&);

	std::vector<Node> nodes;
	std::vector<Undo> undos;
};