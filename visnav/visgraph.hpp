#include "../utils/geom.hpp"
#include <vector>

class Image;

struct VisGraph {
	VisGraph(std::vector<Polygon> &ps) : polys(ps) {
		computegraph();
	}

	void draw(Image&) const;

private:

	void computegraph(void);
	void computeverts(void);
	void linkverts(void);
	void linkvert(unsigned int vid);
	void addedge(unsigned int, unsigned int, double);
	bool consecutive(unsigned int, unsigned int);	// On same poly-side?

	struct Vert;

	struct Edeg {
		Edeg(Vert *_dest, double _dist) : dest(_dest), dist(_dist) { }
		Vert *dest;
		double dist;
	};

	struct Vert {
		Vert(unsigned int _vid, const Point &_pt, unsigned int _polyno,
				unsigned int _vertno) :
			pt(_pt), vid(_vid), polyno(_polyno), vertno(_vertno) { }

		Point pt;
		unsigned int vid;
		unsigned int polyno, vertno;
		std::vector<Edeg> succs;
	};

	std::vector<Polygon> polys;
	std::vector<Vert> verts;
};