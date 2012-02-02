#include "visnav.hpp"
#include "../utils/image.hpp"

const VisNav::Oper VisNav::Nop;

VisNav::VisNav(const VisGraph &_g, double _x0, double _y0, double _x1, double _y1) :
		x0(_x0), y0(_y0), x1(_x1), y1(_y1), g(_g) {
	start = g.add(Geom2d::Point(x0, y0));
	finish = g.add(Geom2d::Point(x1, y1));
}

VisNav::State VisNav::initialstate(void) {
	return State(start);
}

enum {
	Width = 400,
	Height = 400,
};

void VisNav::save(const char *file, std::vector<State> path) {
	Image img(Width, Height);

	Geom2d::Point min = g.min();
	Geom2d::Point max = g.max();
	g.translate(-min.x, -min.y);
	double w = max.x - min.x;
	double h = max.y - min.y;
	double sx = Width / w, sy = Height / h;
	if (sx < sy)
		g.scale(sx, sx);
	else
		g.scale(sy, sy);

	g.draw(img, false);

	Geom2d::Point p0 = g.verts[path[0].vert].pt;
	for (unsigned int i = 1; i < path.size(); i++) {
		const Geom2d::Point &p1 = g.verts[path[i].vert].pt;
		img.add(new Image::Line(p0, p1, Image::red, 3));
		p0 = p1;
	}

	img.saveeps(file, true, 72.0/2.0);
}