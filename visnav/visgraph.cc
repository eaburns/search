#include "visgraph.hpp"
#include "../utils/image.hpp"
#include "../utils/utils.hpp"
#include <cerrno>

VisGraph::VisGraph(FILE *in) : PolyMap(in) {
	unsigned int nverts;
	int res = fscanf(in, " %u vertices\n", &nverts);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the visibility graph");
	if (res != 1)
		fatal("Malformed visibility graph");

	for (unsigned int i = 0; i < nverts; i++)
		verts.push_back(Vert(in));
}

VisGraph::VisGraph(const PolyMap &p) : PolyMap(p) {
	build();
}

void VisGraph::output(FILE *out) const {
	PolyMap::output(out);

	fprintf(out, "%u vertices\n", (unsigned int) verts.size());
	for (unsigned int i = 0; i < verts.size(); i++) {
		verts[i].output(out);
		putc('\n', out);
	}
}

VisGraph::Edge::Edge(FILE *in) {
	int res = fscanf(in, " %u %u %lg", &src, &dst, &dist);
	if (res == EOF)
		fatalx(errno, "Failed to read the edeg");
	if (res != 3)
		fatal("Malformed edge");
}

void VisGraph::Edge::output(FILE *out) const {
	fprintf(out, "%u %u %g", src, dst, dist);
}

VisGraph::Vert::Vert(FILE *in) {
	unsigned int nedges;
	int res = fscanf(in, " %u %lg %lg %u", &id, &pt.x, &pt.y, &nedges);
	if (res == EOF)
		fatalx(errno, "Failed to read the vertex");
	if (res != 4)
		fatal("Malformed vertex");

	for (unsigned int i = 0; i < nedges; i++)
		edges.push_back(Edge(in));
}

void VisGraph::Vert::output(FILE *out) const {
	fprintf(out, "%u %g %g %u", id, pt.x, pt.y, (unsigned int) edges.size());
	for (unsigned int i = 0; i < edges.size(); i++) {
		putc(' ', out);
		edges[i].output(out);
	}
}

void VisGraph::draw(Image &img, bool label) const {
	static const double polywidth = 1;
	PolyMap::draw(img, polywidth);

	for (unsigned int i = 0; i < verts.size(); i++) {
	for (unsigned int j = 0; j < verts[i].edges.size(); j++) {
		static const Color gray(0.75, 0.75, 0.75);
		static const double lwidth = 0.2;
		const Point &a = verts[i].pt;
		const Point &b = verts[verts[i].edges[j].dst].pt;
		img.add(new Image::Line(a.x, a.y, b.x, b.y, lwidth, gray));
	}
	}

	for (unsigned int i = 0; i < verts.size(); i++) {
		static const double radius = 1;
		const Point &p = verts[i].pt;
		p.draw(img, Image::black, radius);

		if (!label)
			continue;

		// If we want vertices labeled then we also
		// want to know the location of each in exact
		// coordinates.
		char buf[128];
		static const double txtsz = 8;
		snprintf(buf, sizeof(buf), "%u", i);
		img.add(new Image::Text(buf, p.x, p.y + radius, txtsz));
	}
}

void VisGraph::dumpvertlocs(FILE *out) const {
	for (unsigned int i = 0; i < verts.size(); i++)
		fprintf(out, "%u: %g, %g\n", i, verts[i].pt.x, verts[i].pt.y);
}

void VisGraph::scale(double sx, double sy) {
	PolyMap::scale(sx, sy);
	for (unsigned int i = 0; i < verts.size(); i++)
		verts[i].pt.scale(sx, sy);

	for (unsigned int i = 0; i < verts.size(); i++) {
		const Point &src = verts[i].pt;
		for (unsigned int j = 0; j < verts[i].edges.size(); j++) {
			unsigned int dstid = verts[i].edges[j].dst;
			const Point &dst = verts[dstid].pt;
			verts[i].edges[j].dist =Point::distance(src, dst);
		}
	}
}

void VisGraph::translate(double dx, double dy) {
	PolyMap::translate(dx, dy);
	for (unsigned int i = 0; i < verts.size(); i++)
		verts[i].pt.translate(dx, dy);
}

unsigned int VisGraph::add(const Point &pt) {
	unsigned int vid = verts.size();
	verts.push_back(Vert(vid, pt));
	for (unsigned int i = 0; i < vid; i++)
		consideredge(i, vid);
	return vid;
}

void VisGraph::build(void) {
	populateverts();
	visedges();
}

void VisGraph::populateverts(void) {
	std::vector<unsigned int> vs;
	for (unsigned int i = 0; i < polys.size(); i++) {
		vs.clear();
		for (unsigned int j = 0; j < polys[i].verts.size(); j++) {
			if (polys[i].isreflex(j))
				vs.push_back(j);
		}
		addpoly(polys[i], vs);
		for (unsigned int j = 0; j < vs.size(); j++)
			polyno.push_back(i);
	}
	if (bound) {		vs.clear();
		for (unsigned int i = 0; i < bound->verts.size(); i++) {
			if (bound->isreflex(i))
				vs.push_back(i);
		}
		addpoly(*bound, vs);
	}
}

void VisGraph::addpoly(const Polygon &p, const std::vector<unsigned int> &vs) {
	unsigned int id0 = verts.size();

	// add all reflex vertices for this polygon
	for (unsigned int j = 0; j < vs.size(); j++) {
		const Point &pt = p.verts[vs[j]];
		verts.push_back(Vert(verts.size(), pt));
	}

	// link adjacent vertices
	unsigned int n = p.verts.size();
	for (unsigned int i = 0; i < vs.size(); i++) {
	for (unsigned int j = i + 1; j < vs.size(); j++) {
		if ((vs[i] + 1) % n == vs[j] || (vs[j] + 1) % n == vs[i])
			addedge(id0 + i, id0 + j);
	}
	}
}

void VisGraph::visedges(void) {
	for (unsigned int i = 0; i < verts.size() - 1; i++) {
	for (unsigned int j = i + 1; j < verts.size(); j++)
		consideredge(i, j);
	}
}

void VisGraph::consideredge(unsigned int i, unsigned int j) {
	LineSeg ray(verts[i].pt, verts[j].pt);
	double len = ray.length();

	Point p0(ray.along(len * 1e-3));
	if (i < polyno.size() && polys[polyno[i]].contains(p0))
		return;

	Point p1(ray.along(len * (1 - 1e-3)));
	if (j < polyno.size() && polys[polyno[j]].contains(p1))
		return;

	if (isvisible(p0, p1))
		addedge(i, j);
}

void VisGraph::addedge(unsigned int i, unsigned int j) {
	double dist = Point::distance(verts[i].pt, verts[j].pt);
	verts[i].edges.push_back(Edge(i, j, dist));
	verts[j].edges.push_back(Edge(j, i, dist));
}