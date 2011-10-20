#include "visgraph.hpp"
#include "../utils/image.hpp"

enum { Bufsz = 10 };

void VisGraph::draw(Image &img) const {
	unsigned int nextcolor = 0;
	for (unsigned int i = 0; i < polys.size(); i++) {
		const Color &c = somecolors[nextcolor];
		nextcolor = (nextcolor+1) % Nsomecolors;
		polys[i].draw(img, c, 3);
	}

	for (unsigned int i = 0; i < verts.size(); i++) {
		for (unsigned int j = 0; j < verts[i].succs.size(); j++) {
			unsigned int dst = verts[i].succs[j].dest->vid;
			img.add(new Image::Line(verts[i].pt.x, verts[i].pt.y,
				verts[dst].pt.x, verts[dst].pt.y, 0.5, Color(0.75, 0.75, 0.75)));
		}
	}

	for (unsigned int i = 0; i < verts.size(); i++)
		img.add(new Image::Circle(verts[i].pt.x, verts[i].pt.y, 3));
}

void VisGraph::computegraph(void) {
	computeverts();
	linkverts();
}

void VisGraph::computeverts(void) {
	for (unsigned int i = 0; i < polys.size(); i++) {
	for (unsigned int j = 0; j < polys[i].verts.size(); j++) {
		if (!polys[i].isreflex(j))
			continue;
		verts.push_back(Vert(verts.size(), polys[i].verts[j], i, j));
	}
	}
}

void VisGraph::linkverts(void) {
	// No need to link the last vertex.  All links are
	// undirected so it will be linked by all of the
	// other verts.

	for (unsigned int i = 0; i < verts.size() - 1; i++)
		linkvert(i);
}

// Links the vertex vid with all vertices with greater
// ID values.
void VisGraph::linkvert(unsigned int i) {
	static const double Epsilon = 1e-5;

	for (unsigned int j = i+1; j < verts.size(); j++) {
		if (consecutive(i, j)) {
			addedge(i, j, Point::distance(verts[i].pt, verts[j].pt));
			continue;
		}

		Line line(verts[i].pt, verts[j].pt);

		if (verts[i].polyno == verts[j].polyno) {
			Point mid = line.midpoint();
			if (polys[verts[i].polyno].contains(mid))
				continue;
		}

		double len = line.length();
		bool vis = true;
		for (unsigned int p = 0; p < polys.size(); p++) {
			double hit = polys[p].minhit(line, p == verts[i].polyno ? Epsilon : 0.0);
			if (hit > Epsilon && hit < len - Epsilon) {
				vis = false;
				break;
			}
		}
		if (vis)
			addedge(i, j, len);
	}
}

bool VisGraph::consecutive(unsigned int i, unsigned int j) {
	if (verts[i].polyno != verts[j].polyno)
		return false;

	unsigned int n = polys[verts[i].polyno].verts.size();
	unsigned int u = verts[i].vertno, v = verts[j].vertno;
	return abs(u - v) == 1 || (u == 0 && v == n - 1) || (u == n - 1 && v == 0);
}

void VisGraph::addedge(unsigned int i, unsigned int j, double len) {
	verts[i].succs.push_back(Edeg(&verts[j], len));
	verts[j].succs.push_back(Edeg(&verts[i], len));
}