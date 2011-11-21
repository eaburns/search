#include "../utils/geom.hpp"
#include <vector>
#include <cstdio>

class Image;

// A multiple-layer visibility graph.  Each z-layer is
// composed of a bunch of polygons in 2d space.  The
// vertices of each layer are linked based on visibility.
// Additional vertices and links can be added to link the
// different layers too.
struct VisGraph {
	VisGraph(void) { }

	VisGraph(std::vector<Polygon> &ps) {
		layers.push_back(ps);
		computegraph();
	}

	VisGraph(FILE*);

	void draw(Image&, double scale=1, unsigned int z = 0) const;

	void output(FILE*) const;

	// Add a layer and return its z-value.
	unsigned int pushlayer(std::vector<Polygon>&);

	// Add a vertex to the graph.
	unsigned int add(double, double, unsigned int z = 0);

	// Link two vertices.
	void link(unsigned int, unsigned int, double);

	// Is the given point outside all polygons?
	bool isoutside(double, double, unsigned int z = 0);

	struct Vert;

	struct Edge {
		Edge(unsigned int _src, unsigned int _dst, double _dist) :
			src(_src), dst(_dst), dist(_dist) { }
		unsigned int src, dst;
		double dist;
	};

	struct Vert {
		Vert(void) { }

		Vert(unsigned int _vid, const Point &_pt, unsigned int _z, unsigned int _polyno,
				unsigned int _vertno) :
			pt(_pt), vid(_vid), z(_z), polyno(_polyno), vertno(_vertno) { }

		void input(std::vector<Vert>&, unsigned int, FILE*);
		void output(FILE *out) const;

		Point pt;
		unsigned int vid;
		unsigned int z, polyno, vertno;	// polyno within z-layer
		std::vector<Edge> succs;
	};

	const Vert &vertex(unsigned int i) const { return verts[i]; }

private:
	void computegraph(void);
	void computeverts(unsigned int);
	void linkverts(unsigned int);
	void linkvert(unsigned int, unsigned int);
	bool consecutive(unsigned int, unsigned int);	// On same poly-side?

	std::vector< std::vector<Polygon> > layers;
	std::vector<Vert> verts;
};