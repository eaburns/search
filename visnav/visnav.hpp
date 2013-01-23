#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include <cstdio>

struct VisNav {
	typedef double Cost;

	struct Oper {
		Oper() : edge(NULL) { }
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
		State() { }

		State(int v) : vert(v) { }

		bool eq(const VisNav*, const State &o) const {
			return vert == o.vert;
		}	
	
		unsigned long hash(const VisNav*) const {
			return vert;
		}

	private: 
		friend struct Undo;
		friend struct VisNav;
		int vert;
	};

	typedef State PackedState;

	State initialstate();

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

	struct Operators {
		Operators(const VisNav &d, const State &s) :
			edges(d.g.verts[s.vert].edges) { }

		unsigned int size() const {
			return edges.size();
		}

		Oper operator[](unsigned int i) const {
			return Oper(&edges[i]);
		}

	private:
		const std::vector<VisGraph::Edge> &edges;
	};

	struct Edge {
		Cost cost;
		Oper revop;
		Cost revcost;
		State state;

		Edge(VisNav &d, State &s, Oper op) :
			cost(op.edge->dist), revop(op), revcost(op.edge->dist), state(op.edge->dst) { }
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

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

	void save(const char*, std::vector<State> path = std::vector<State>());

private:
	double x0, y0, x1, y1;
	unsigned int start, finish;
	VisGraph g;
};