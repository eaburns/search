#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include <cstdio>

struct VisNav {
	enum { UnitCost = false };

	typedef double Cost;
	static const double InfCost = -1;

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

		unsigned long hash(void) { return vert; }

		bool eq(const State &o) { return vert == o.vert; }

	private: 
		friend class Undo;
		friend class VisNav;
		int vert;
	};

	typedef State PackedState;

	struct Undo {
		Undo(State &s, Oper op) { vert = (int) op.edge->src; }
		int vert;
	};

	State initialstate(void);

	Cost h(State &s) {//
		double dx = x1 - g.vertex(s.vert).pt.x;
		double dy = y1 - g.vertex(s.vert).pt.y;
		return sqrt(dx * dx + dy * dy);
	}

	bool isgoal(State &s) {
		assert (s.vert >= 0);
		return (unsigned int) s.vert == finish;
	}

	unsigned int nops(State &s) {
		return g.vertex(s.vert).succs.size();
	}

	Oper nthop(State &s, unsigned int n) {
		return Oper(&g.vertex(s.vert).succs[n]);
	}

	Oper revop(State &s, Oper op) {
		return op;
	}

	Cost opcost(State &s, Oper op) {
		return op.edge->dist;
	}

	void undo(State &s, Undo &u) {
		s.vert = u.vert;
	}

	State &apply(State &buf, State &s, Oper op) {
		s.vert = op.edge->dst;
		return s;
	}

	void pack(PackedState &dst, State &src) {
		dst = src;
	}

	State &unpack(State &buf, PackedState &pkd) {
		return pkd;
	}

	void dumpstate(FILE *out, State &s) {
		fprintf(out, "%d\n", s.vert);
	}

	void save(const char*, std::vector<State> path = std::vector<State>()) const;

private:
	double x0, y0, x1, y1;
	unsigned int start, finish;
	VisGraph g;
};