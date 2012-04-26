#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include <cstdio>

struct VisNav {
	typedef double Cost;

	struct Oper {
		Oper(void) : edge(NULL) { }
		Oper(const VisGraph::Edge *e) : edge(e) { }
		bool operator==(const Oper &other) const {
			if (!edge)
				return !other.edge;
			if (!other.edge)
				return false;
			return other.edge->src == edge->src || other.edge->dst == edge->dst;
		}
		const VisGraph::Edge *edge;
	};

	static const Oper Nop;

	VisNav(const VisGraph&, double x0, double y0, double x1, double y1);

	struct Undo;

	struct State {
		State(void) { }

		State(int v) : vert(v) { }

		bool operator==(const State &o) const {
			return vert == o.vert;
		}

	private: 
		friend struct Undo;
		friend struct VisNav;
		int vert;
	};

	typedef State PackedState;

	State initialstate(void);

	unsigned long hash(PackedState &p) {
		return p.vert;
	}

	Cost h(State &s) {
		const geom2d::Pt &pt = g.verts[s.vert].pt;
		double dx = x1 - pt.x;
		double dy = y1 - pt.y;
		return sqrt(dx * dx + dy * dy);
	}

	Cost d(State &s) {
		return 0.0;	// for now, I guess
	}

	bool isgoal(State &s) {
		assert (s.vert >= 0);
		return (unsigned int) s.vert == finish;
	}

	unsigned int nops(State &s) {
		return g.verts[s.vert].edges.size();
	}

	Oper nthop(State &s, unsigned int n) {
		return Oper(&g.verts[s.vert].edges[n]);
	}

	struct Edge {
		Cost cost;
		Oper revop;
		State state;

		Edge(VisNav &d, State &s, Oper op) :
			cost(op.edge->dist), revop(op), state(op.edge->dst) { }
	};

	void pack(PackedState &dst, State &src) {
		dst = src;
	}

	State &unpack(State &buf, PackedState &pkd) {
		return pkd;
	}

	void dumpstate(FILE *out, State &s) {
		fprintf(out, "%d\n", s.vert);
	}

	void save(const char*, std::vector<State> path = std::vector<State>());

private:
	double x0, y0, x1, y1;
	unsigned int start, finish;
	VisGraph g;
};