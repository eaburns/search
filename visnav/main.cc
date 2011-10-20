#include "visgraph.hpp"
#include "../utils/utils.hpp"
#include "../utils/image.hpp"

enum {
	Width = 500,
	Height = 500,
};

enum { Step = 10 };

Point pointalong(Line&, double);
void addline(Image&, Line&, Color c = Image::black);
void addaxes(Image&);

int main(int argc, char *argv[]) {
	printf("seed=%lu\n", randgen.seed());

	Poly p1 = Poly::random(6, Width / 4, Height / 4, 50);

	std::vector<Poly> polys;
	for (unsigned int i = 0; i < 100; i++) {
redo:
		unsigned long nverts = randgen.integer(3, 8);
		double x = randgen.real() * Width;
		double y = randgen.real() * Height;
		double r = randgen.real() * 50 + 50;
		Poly p = Poly::random(nverts, x, y, r);
		for (unsigned int j = 0; j < p.verts.size(); j++) {
			unsigned int nxt = j == p.verts.size() - 1 ? 0 : j + 1;
			if (p.verts[j].x > Width || p.verts[j].x < 0)
				goto redo;
			if (p.verts[j].y > Height || p.verts[j].y < 0)
				goto redo;

			for (unsigned int k = 0; k < polys.size(); k++) {
				if (polys[k].contains(p.verts[j]))
					goto redo;

				Line side(p.verts[j], p.verts[nxt]);
				if (!isinf(polys[k].minhit(side)))
					goto redo;
			}
		}
		polys.push_back(p);
	}

	VisGraph graph(polys);

	Image img(Width, Height, "poly.eps");
	graph.draw(img);
	img.save("polys.eps", true);

	return 0;
}

void addline(Image &img, Line &line, Color c) {
	Image::Path *p = new Image::Path();
	p->setlinewidth(0.1);
	p->setcolor(c);
	p->line(line.p0.x, line.p0.y, line.p1.x, line.p1.y);
	img.add(p);
}
