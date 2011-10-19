#include "poly.hpp"
#include "../utils/image.hpp"

void addaxes(Image&);

int main(int argc, char *argv[]) {
	Image img(500, 500, "poly.eps");
	addaxes(img);

//	Poly t0 = Poly::triangle(250, 250, 10);
//	t0.draw(img, Image::black, 0.5);
//
//	Poly sq = Poly::square(250, 250, 20);
//	sq.draw(img, Image::blue, 0.5);

	Poly rand = Poly::random(6, 250, 250, 50);
	rand.draw(img, Image::green, 0.5);

	img.save("polys.eps");

	return 0;
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