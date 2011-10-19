#include "poly.hpp"
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

	Image img(Width, Height, "poly.eps");
	addaxes(img);

	Poly rand = Poly::random(6, Width / 2, Height / 2, 50);
	rand.draw(img, Image::green, 0.5);

	for (double x = 0; x <= Width; x += Step) {
		Line line(0, 0, x, Height);
		double hitdist = rand.minhit(line);
 		if (isinf(hitdist)) {
			addline(img, line);
			continue;
		}
		Point hitpt = pointalong(line, hitdist);
		Line clamped(Point(0, 0), hitpt);
		addline(img, clamped, Image::green);
	}

	for (double y = 0; y <= Height; y += Step) {
		Line line(0, 0, Width, y);
		double hitdist = rand.minhit(line);
 		if (isinf(hitdist)) {
			addline(img, line);
			continue;
		}
		Point hitpt = pointalong(line, hitdist);
		Line clamped(Point(0, 0), hitpt);
		addline(img, clamped, Image::green);
	}

	img.save("polys.eps");

	return 0;
}

Point pointalong(Line &line, double dist) {
	return Point(line.p0.x + dist * cos(line.theta),
			line.p0.y + dist * sin(line.theta));
}

void addline(Image &img, Line &line, Color c) {
	Image::Path *p = new Image::Path();
	p->setlinewidth(0.1);
	p->setcolor(c);
	p->line(line.p0.x, line.p0.y, line.p1.x, line.p1.y);
	img.add(p);
}

void addaxes(Image &img) {
	Color gray(0.5, 0.5, 0.5);
	Image::Path *xaxis = new Image::Path();
	xaxis->setcolor(gray);
	xaxis->setlinewidth(0.5);
	xaxis->moveto(0, img.height() / 2);
	xaxis->lineto(img.width(), img.height() / 2);
	img.add(xaxis);

	Image::Path *yaxis = new Image::Path();
	yaxis->setcolor(gray);
	yaxis->setlinewidth(0.5);
	yaxis->moveto(img.width() / 2, 0);
	yaxis->lineto(img.width() / 2, img.height());
	img.add(yaxis);
}