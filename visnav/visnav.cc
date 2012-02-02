#include "visnav.hpp"
#include "../utils/image.hpp"

const VisNav::Oper VisNav::Nop;

VisNav::VisNav(const VisGraph &_g, double _x0, double _y0, double _x1, double _y1) :
		x0(_x0), y0(_y0), x1(_x1), y1(_y1), g(_g) {
	start = g.add(Geom::Point(x0, y0));
	finish = g.add(Geom::Point(x1, y1));
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

	Geom::Point min = g.min();
	Geom::Point max = g.max();
	g.translate(-min.x, -min.y);
	double w = max.x - min.x;
	double h = max.y - min.y;
	double sx = Width / w, sy = Height / h;
	if (sx < sy)
		g.scale(sx, sx);
	else
		g.scale(sy, sy);

	g.draw(img, false);

	Image::Path *p = new Image::Path();
	p->setlinejoin(Image::Path::Round);
	p->setlinewidth(3);
	p->setcolor(Image::red);
	p->moveto(g.verts[path[0].vert].pt.x,
		g.verts[path[0].vert].pt.y);
	for (unsigned int i = 1; i < path.size(); i++) {
		p->lineto(g.verts[path[i].vert].pt.x,
			g.verts[path[i].vert].pt.y);
	}
	img.add(p);

	img.save(file, true, 72.0/2.0);
}