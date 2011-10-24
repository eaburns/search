#include "visgraph.hpp"
#include "../utils/image.hpp"

enum { Bufsz = 10 };

void VisGraph::draw(Image &img) const {
	static const unsigned int Vradius = 2;
	unsigned int nextcolor = 0;
	for (unsigned int i = 0; i < polys.size(); i++) {
		const Color &c = somecolors[nextcolor];
		nextcolor = (nextcolor+1) % Nsomecolors;
		polys[i].draw(img, c, 2);

//		for (unsigned int j = 0; j < polys[i].verts.size(); j++) {
//			char buf[Bufsz];
//			snprintf(buf, Bufsz, "%u", j);
//			img.add(new Image::Text(buf, polys[i].verts[j].x,
//				polys[i].verts[j].y + Vradius + 1, 12));
//		}
	}

	for (unsigned int i = 0; i < verts.size(); i++) {
		for (unsigned int j = 0; j < verts[i].succs.size(); j++) {
			unsigned int dst = verts[i].succs[j].dest->vid;
			img.add(new Image::Line(verts[i].pt.x, verts[i].pt.y,
				verts[dst].pt.x, verts[dst].pt.y, 0.2, Color(0.75, 0.75, 0.75)));
		}
	}

	for (unsigned int i = 0; i < verts.size(); i++)
		verts[i].pt.draw(img, Image::black, Vradius);
}

void VisGraph::computegraph(void) {
	computeverts();
	linkverts();
}

void VisGraph::computeverts(void) {
	for (unsigned int polyno = 0; polyno < polys.size(); polyno++) {
		std::vector<unsigned int> rs;
		Polygon &p = polys[polyno];
		p.reflexes(rs);
		for (unsigned int i = 0; i < rs.size(); i++) {
			unsigned int vertno = rs[i];
			unsigned int vid = verts.size();
			verts.push_back(Vert(vid, p.verts[vertno], polyno, vertno));
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

		LineSeg line(verts[i].pt, verts[j].pt);
		line = LineSeg(line.along(Epsilon), line.p1);

		if (verts[i].polyno == verts[j].polyno) {
			Point mid = line.midpt();
			if (polys[verts[i].polyno].contains(mid))
				continue;
		}

		double len = line.length();
		bool vis = true;
		for (unsigned int p = 0; p < polys.size(); p++) {
			Point hit = polys[p].minisect(line);
			double dist = Point::distance(verts[i].pt, hit);
			if (dist < len - Epsilon) {
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