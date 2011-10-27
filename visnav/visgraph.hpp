#include "../utils/geom.hpp"
#include <vector>
#include <cstdio>

class Image;

struct VisGraph {
	VisGraph(std::vector<Polygon> &ps) : polys(ps) {
		computegraph();
	}

	VisGraph(FILE*);

	void draw(Image&, double scale=1) const;
	void output(FILE*) const;

private:

	void computegraph(void);
	void computeverts(void);
	void linkverts(void);
	void linkvert(unsigned int vid);
	void addedge(unsigned int, unsigned int, double);
	bool consecutive(unsigned int, unsigned int);	// On same poly-side?

	struct Vert;

	struct Edge {
		Edge(Vert *_dest, double _dist) : dest(_dest), dist(_dist) { }
		Vert *dest;
		double dist;

		friend class Vert;
	};

	struct Vert {
		Vert(void) { }

		Vert(unsigned int _vid, const Point &_pt, unsigned int _polyno,
				unsigned int _vertno) :
			pt(_pt), vid(_vid), polyno(_polyno), vertno(_vertno) { }

		void input(std::vector<Vert>&, unsigned int, FILE*);
		void output(FILE *out) const;

		Point pt;
		unsigned int vid;
		unsigned int polyno, vertno;
		std::vector<Edge> succs;
	};

	std::vector<Polygon> polys;
	std::vector<Vert> verts;
};