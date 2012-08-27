#include "../utils/utils.hpp"
#include "../utils/geom2d.hpp"
#include <cstdio>
#include <vector>
#include <cmath>
#include <utility>

struct Segments {
private:
	friend void mkinst(FILE*);

	// A Pose gives the pose of a particular segment.
	struct Pose {
		Pose() { }

		Pose(unsigned int xx, unsigned int yy, unsigned int r) : x(xx), y(yy), rot(r) {
		}

		bool operator==(const Pose &o) const {
			return x == o.x && y == o.y &&  rot == o.rot;
		}

		bool operator!=(const Pose &o) const {
			return !(*this == o);
		}

		unsigned int x, y, rot;
	};

	// Angle contains information about angles.
	struct Angle {
		Angle(double);

		double theta;
		double sine;
		double cosine;
	};

	// A Sweep contains the geometry primitives that
	// represent a segment sweeping through space.
	struct Sweep {
		// hits returns true if the line segment hits
		// the sweep;
		bool hits(const geom2d::LineSg&) const;

		unsigned int nlines;
		geom2d::LineSg lines[3];

		unsigned int narcs;
		geom2d::Arc arcs[2];
	};

public:
	// Seg contains segment specific information.
	struct Seg {
		double radius;
		Pose goal;
		Pose start;
	};

	Segments(FILE*);

	Segments(unsigned int w, unsigned int h, unsigned int t, const std::vector<Seg>&);

	struct PackedState;

	struct State {
		State() { }

		// This constructor creates a state from the given poses.
		State(const Segments &dom, const std::vector<Pose> &ps,
			unsigned int xx, unsigned int yy) : x(xx), y(yy) {
			init(dom, ps);
		}

		State(const Segments &dom, const PackedState &pkd) : x(pkd.x), y(pkd.y) {
			init(dom, pkd.poses);
		}

		State(const State &o) : x(o.x), y(o.y), poses(o.poses), lines(o.lines), nleft(o.nleft) { }

		State(State &&o) : x(o.x), y(o.y), nleft(o.nleft) {
			std::swap(poses, o.poses);
			std::swap(lines, o.lines);
		}

		State& operator=(const State &o) {
			x = o.x;
			y = o.y;
			nleft = o.nleft;
			poses = o.poses;
			lines = o.lines;
			return *this;
		}

		State& operator=(State &&o) {
			x = o.x;
			y = o.y;
			nleft = o.nleft;
			std::swap(poses, o.poses);
			std::swap(lines, o.lines);
			return *this;
		}

		bool operator==(const State&) const;

		// x and y are the gripper's position.
		unsigned int x, y;

		// poses gives the pose of each segment.
		std::vector<Pose> poses;

		// lines gives the line segment representing
		// each segment.
		std::vector<geom2d::LineSg> lines;

		// nleft is the number of segments that still
		// need to be moved to their goal location.
		unsigned int nleft;

	private:
		// init initializes the poses, lines, and nleft fields of a
		// state from a vector of poses.
		void init(const Segments&, const std::vector<Pose>&);
	};

	struct PackedState {
		PackedState() { }

		PackedState(const State &s) : poses(s.poses), x(s.x), y(s.y) {
		}

		PackedState(PackedState &&o) : poses(std::move(o.poses)), x(o.x), y(o.y) {
		}

		PackedState &operator=(PackedState &&o) {
			x = o.x;
			y = o.y;
			std::swap(poses, o.poses);
			return *this;
		}

		bool operator==(const PackedState &o) const {
			for (unsigned int i = 0; i < poses.size(); i++) {
				if (poses[i] != o.poses[i])
					return false;
			}
			return x == o.x && y == o.y;
		}

		std::vector<Pose> poses;
		unsigned int x, y;
	};

	typedef double Cost;

	struct Oper {
		enum Op { None = 0, Gripper, Rotate, Move };

		Oper() { }

		Oper(Op o, unsigned int s, int d, int y) : op(o), seg(s), delta(d), dy(y) {
		}

		// Equality test.
		//
		// All Gripper operations are equal regardless
		// of their seg fields.
		bool operator==(const Oper &o) const;

		// reverse returns the operator that, if applied
		// to the resulting state, would undo this operator.
		//
		// The reverse of a Gripper operation is the same
		// operation, this works because all Gripper ops
		// are equivalent.  This means that the Gripper is
		// never moved twice in a row.
		Oper reverse() const;

		// cost returns the cost of applying this operator
		// in the given state.
		Cost cost(const State&) const;

		// sweep returns the Sweep when applying
		// this operator in the given state.
		Sweep sweep(const Segments&, const State&) const;

		// ok returns true if the operator is applicable
		// in the given state.
		bool ok(const Segments&, const State&) const;

		// op is the operation.
		Op op;

		// seg is the target of the operation.
		unsigned int seg;

		// delta is either the change in x or in rotation.
		int delta, dy;
	};

	static Oper Nop;

	struct Operators {
		Operators(const Segments&, const State&);

		unsigned int size() const {
			return ops.size();
		}

		Oper operator[](unsigned int i) const {
			return ops[i];
		}

		std::vector<Oper> ops;
	};

	struct Edge {
		Cost cost;
		Oper revop;
		State state;

		Edge(const Segments&, const State&, Oper);

		~Edge() { }
	};

	State initialstate() const;

	unsigned long hash(const PackedState&) const;

	Cost h(const State&) const;

	Cost d(const State&) const;

	bool isgoal(const State &s) const {
		return s.nleft == 0;
	}

	void pack(PackedState &dst, const State &src) const {
		dst = PackedState(src);
	}

	State &unpack(State &buf, const PackedState &pkd) const {
		buf = State(*this, pkd);
		return buf;
	}

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

	// prinitial prints the initial state to the given
	// file in datafile format.
	void prinitial(FILE*) const;
	unsigned int width, height, nangles;
private:

	// line returns the line for the given segment
	// in the given pose.
	geom2d::LineSg line(const Seg&, const Pose&) const;



	// segs is the segments.
	std::vector<Seg> segs;

	// angles holds information about the different angles
	// at which a segment can be rotated.
	std::vector<Angle> angles;

	// bounds are the lines defining the boundaries on the
	// working space of the segments.
	geom2d::LineSg bounds[4];
};

// scanops scans an operator vector from an
// operator string. 
std::vector<Segments::Oper> scanops(const std::string&);

struct Solution {
	int width, height, nangles;
	std::vector<Segments::Seg> segs;
	std::vector<Segments::Oper> ops;
};

// readdf reads the segments instance and
// operators from a datafile.  If echo is
// non-null then each line of the datafile is
// echoed to the given FILE*.
Solution readdf(FILE *in, FILE *echo);
