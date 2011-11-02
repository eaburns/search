#include "visnav.hpp"
#include "../utils/image.hpp"

const VisNav::Oper VisNav::Nop;

VisNav::VisNav(const VisGraph &_g, double _x0, double _y0, double _x1, double _y1) :
		x0(_x0), y0(_y0), x1(_x1), y1(_y1), g(_g) {
	start = g.add(x0, y0);
	finish = g.add(x1, y1);
}

VisNav::State VisNav::initialstate(void) {
	return State(start);
}

void VisNav::save(const char *file, std::vector<State> path) const {
	static const unsigned int scale = 400;

	Image img(scale, scale);
	g.draw(img, scale);

	Image::Path *p = new Image::Path();
	p->setlinewidth(3);
	p->setcolor(Image::red);
	p->moveto(g.vertex(path[0].vert).pt.x * scale,
		g.vertex(path[0].vert).pt.y * scale);
	for (unsigned int i = 1; i < path.size(); i++) {
		p->lineto(g.vertex(path[i].vert).pt.x * scale,
			g.vertex(path[i].vert).pt.y * scale);
	}
	img.add(p);

	img.save(file, true, 72.0/2.0);
}