#include "polymap.hpp"
#include <vector>
#include <cstdio>

struct Image;

// A VisGraph is a visibility map along with edges
// between pairs of visible vertices.
struct VisGraph : public PolyMap {

	VisGraph(const PolyMap&);

	VisGraph(FILE*);

	// output writes the visibility graph to the
	// given file.
	void output(FILE*) const;

	// dumpvertlocs writes the location of each vertex
	// to the given file.  This is mostly for debugging.
	void dumpvertlocs(FILE*) const;

	// draw draws the visibility graph to the give
	// image.  If label is true then each vertex
	// is labeled with its ID number.
	void draw(Image&, bool label = false) const;

	// scale scales the visibility graph by the
	// given factors in both the x and y directions.
	void scale(double, double);

	// translate translates the visibility graph by the
	// give x and y values.
	void translate(double, double);

	// add adds a vertex to the graph.
	unsigned int add(const Geom2d::Point&);

	struct Edge {
		Edge(unsigned int s, unsigned int d, double c) :
 			src(s), dst(d), dist(c) { }

		Edge(FILE*);

		void output(FILE*) const;

		unsigned int src, dst;
		double dist;
	};

	struct Vert {
		Vert(unsigned int i, const Geom2d::Point &p) : id(i), pt(p) { }

		Vert(FILE*);

		void output(FILE*) const;

		unsigned int id;
 		Geom2d::Point pt;
		std::vector<Edge> edges;
	};

	std::vector<Vert> verts;

private:

	// build bulids the visibility graph for the
	// polygons.
	void build(void);

	// popverts populates the vertex vector and
	// adds edges between adjacent vertices.
	void populateverts(void);

	// addpoly adds the vertices specified by the index vector
	//  for the given polygon to the graph.
	void addpoly(const Geom2d::Polygon&, const std::vector<unsigned int>&);

	// visedges adds edges between each pair of
	// vertices that are visible from eachother.
	void visedges(void);

	// consideredge considers adding an edge between
	// the two vertices specified by their vertex IDs.
	// If an edge should be added then it is.
	void consideredge(unsigned int, unsigned int);

	// addedge adds an edge between the two vertices
	// with the given IDs.
	void addedge(unsigned int, unsigned int);

	std::vector<unsigned int> polyno;
};