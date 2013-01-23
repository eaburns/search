#include "visnav.hpp"
#include "../graphics/image.hpp"

const VisNav::Oper VisNav::Nop;

VisNav::VisNav(const VisGraph &gr, double x0vl, double y0vl, double x1vl, double y1vl) :
		x0(x0vl), y0(y0vl), x1(x1vl), y1(y1vl), g(gr) {
	start = g.add(geom2d::Pt(x0, y0));
	finish = g.add(geom2d::Pt(x1, y1));
}

VisNav::State VisNav::initialstate() {
	return State(start);
}

enum {
	Width = 400,
	Height = 400,
};

void VisNav::save(const char *file, std::vector<State> path) {
	Image img(Width, Height);

	geom2d::Pt min = g.map.min();
	geom2d::Pt max = g.map.max();
	g.translate(-min.x, -min.y);
	double w = max.x - min.x;
	double h = max.y - min.y;
	double sx = Width / w, sy = Height / h;
	if (sx < sy)
		g.scale(sx, sx);
	else
		g.scale(sy, sy);

	g.draw(img, false);

	geom2d::Pt p0 = g.verts[path[0].vert].pt;
	for (unsigned int i = 1; i < path.size(); i++) {
		const geom2d::Pt &p1 = g.verts[path[i].vert].pt;
		img.add(new Image::Line(p0, p1, Image::red, 3));
		p0 = p1;
	}

	img.saveeps(file, true, 72.0/2.0);
}

VisNav::Cost VisNav::pathcost(const std::vector<State> &path, const std::vector<Oper> &ops) {

	State state = initialstate();
	Cost cost(0);
	for (int i = ops.size() - 1; i >= 0; i--) {
		State copy(state);
		Edge e(*this, copy,ops[i]);
		assert (e.state.eq(this, path[i]));
		state = e.state;
		cost += e.cost;
	}
	assert (isgoal(state));
	return cost;
}