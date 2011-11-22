#include "visgraph.hpp"
#include "../utils/image.hpp"
#include "../utils/utils.hpp"
#include <cerrno>

VisGraph::VisGraph(FILE *in) {
	unsigned long npoly;

	unsigned int zs;
	if (fscanf(in, "%u z-layers\n", &zs) != 1)
		fatalx(errno, "Failed to read number of z-layers");

	layers.resize(zs);
	for (unsigned int z = 0; z < zs; z++) {
		if (fscanf(in, " %lu polygons\n", &npoly) != 1)
			fatalx(errno, "Failed to read number of polygons");
	
		for (unsigned long i = 0; i < npoly; i++)
			layers[z].push_back(Polygon(in));
	}

	unsigned long nverts;
	if (fscanf(in, " %lu vertices\n", &nverts) != 1)
		fatalx(errno, "Failed to read the number of vertices");

	verts.resize(nverts);
	for (unsigned long i = 0; i < nverts; i++)
		verts[i].input(verts, i, in);
}

void VisGraph::draw(Image &img, double scale, unsigned int z) const {
	static const unsigned int Vradius = 1;
	unsigned int nextcolor = 0;
	for (unsigned int i = 0; i < layers[0].size(); i++) {
		const Color &c = somecolors[nextcolor];
		nextcolor = (nextcolor+1) % Nsomecolors;
		Polygon p = layers[z][i];
		p.scale(scale);
		p.draw(img, c, 2);
	}

	for (unsigned int i = 0; i < verts.size(); i++) {
		for (unsigned int j = 0; j < verts[i].succs.size(); j++) {
			unsigned int dst = verts[i].succs[j].dst;
			img.add(new Image::Line(
				scale * verts[i].pt.x, scale * verts[i].pt.y,
				scale * verts[dst].pt.x, scale * verts[dst].pt.y,
				0.2, Color(0.75, 0.75, 0.75)));
		}
	}

	for (unsigned int i = 0; i < verts.size(); i++) {
		Point p = verts[i].pt;
		p.x *= scale;
		p.y *= scale;
		p.draw(img, Image::black, Vradius);
	}
}

void VisGraph::output(FILE *out) const {
	fprintf(out, "%u z-layers\n", (unsigned int) layers.size());

	for (unsigned int z = 0; z < layers.size(); z++) {
		fprintf(out, "%lu polygons\n", (unsigned long) layers[z].size());
		for (unsigned int i = 0;  i < layers[z].size(); i++)
			layers[z][i].output(out);
	}

	fprintf(out, "%lu vertices\n", (unsigned long) verts.size());
	for (unsigned int i = 0; i < verts.size(); i++)
		verts[i].output(out);
}

void VisGraph::Vert::output(FILE *out) const {
	fprintf(out, "%g %g %u %u %u %lu", pt.x, pt.y, z, polyno, vertno,
		(unsigned long) succs.size());

	for (unsigned int i = 0; i < succs.size(); i++)
		fprintf(out, " %u %g", succs[i].dst, succs[i].dist);
	fputc('\n', out);
}

void VisGraph::Vert::input(std::vector<Vert> &verts, unsigned int _vid, FILE *in) {
	vid = _vid;

	unsigned long nsuccs;
	if (fscanf(in, " %lg %lg %u %u %u %lu", &pt.x, &pt.y, &z, &polyno, &vertno, &nsuccs) != 6)
		fatalx(errno, "Failed to read a visgraph vertex");

	for (unsigned long i = 0; i < nsuccs; i++) {
		unsigned int dest;
		double dist;
		if (fscanf(in, " %u %lg", &dest, &dist) != 2)
			fatalx(errno, "Failed to read a vertex out-edge");
		succs.push_back(VisGraph::Edge(vid, dest, dist));
	}
}

unsigned int VisGraph::pushlayer(std::vector<Polygon> &ps) {
	unsigned int z = layers.size();
	layers.push_back(ps);
	computeverts(z);
	linkverts(z);
	return z;
}

unsigned int VisGraph::add(double x, double y, unsigned int z) {
	unsigned int vid = verts.size();
	unsigned int polyno = layers[z].size() + vid;
	// Some brandnew polygon that doesn't exist
	verts.push_back(Vert(vid, Point(x, y), z, polyno, vid));
	linkvert(vid, z);
	return vid;
}

bool VisGraph::isoutside(double x, double y, unsigned int z) {
	Point p(x, y);
	for (unsigned int i = 0; i < layers[z].size(); i++) {
		if (layers[z][i].contains(p))
			return false;
	}
	return true;
}

void VisGraph::computegraph(void) {
	for (unsigned int z = 0; z < layers.size(); z++) {
		computeverts(z);
		linkverts(z);
	}
}

void VisGraph::computeverts(unsigned int z) {
	for (unsigned int polyno = 0; polyno < layers[z].size(); polyno++) {
		std::vector<unsigned int> rs;
		Polygon &p = layers[z][polyno];
		p.reflexes(rs);
		for (unsigned int i = 0; i < rs.size(); i++) {
			unsigned int vertno = rs[i];
			unsigned int vid = verts.size();
			verts.push_back(Vert(vid, p.verts[vertno], z, polyno, vertno));
  		}
	}
}

void VisGraph::linkverts(unsigned int z) {
	// No need to link the last vertex.  All links are
	// undirected so it will be linked by all of the
	// other verts.

	for (unsigned int i = 1; i < verts.size(); i++)
		linkvert(i, z);
}

// Links the vertex vid with all vertices with lesser
// ID values.
void VisGraph::linkvert(unsigned int i, unsigned int z) {
	static const double Epsilon = 1e-5;

	for (int j = (int) i-1; j >= 0; j--) {
		if (verts[j].z != z)
			continue;

		if (consecutive(i, j)) {
			link(i, j, Point::distance(verts[i].pt, verts[j].pt));
			continue;
		}

		LineSeg line(verts[i].pt, verts[j].pt);
		line = LineSeg(line.along(Epsilon), line.p1);

		Point mid = line.midpt();
		if (layers[z][verts[i].polyno].contains(mid)
				|| layers[z][verts[j].polyno].contains(mid))
			continue;

		double len = line.length();
		bool vis = true;
		for (unsigned int p = 0; p < layers[z].size(); p++) {
			Point hit = layers[z][p].minisect(line);
			double dist = Point::distance(verts[i].pt, hit);
			if (dist < len - Epsilon) {
				vis = false;
				break;
			}
		}
		if (vis)
			link(i, j, len);
	}
}

bool VisGraph::consecutive(unsigned int i, unsigned int j) {
	if (verts[i].polyno != verts[j].polyno)
		return false;

	unsigned int z = verts[i].z;
	unsigned int n = layers[z][verts[i].polyno].verts.size();
	unsigned int u = verts[i].vertno, v = verts[j].vertno;
	return abs(u - v) == 1 || (u == 0 && v == n - 1) || (u == n - 1 && v == 0);
}

void VisGraph::link(unsigned int i, unsigned int j, double len) {
	verts[i].succs.push_back(Edge(i, j, len));
	verts[j].succs.push_back(Edge(j, i, len));
}