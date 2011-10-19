#include "poly.hpp"
#include "../utils/image.hpp"

void addaxes(Image&);

int main(int argc, char *argv[]) {
	Image img(500, 500, "poly.eps");
	addaxes(img);

	Poly t0(50, 50, 10, 3, 0);
	t0.draw(img, Image::black);

	Poly t1(50, 50, 10, 3, M_PI/4);
	t1.draw(img, Image::green);

	Poly square(75, 75, 20, 4, 0);
	square.draw(img, Image::blue);

	Poly oct0(300, 300, 50, 8, 0);
	oct0.draw(img, Image::blue, 0.1);

	Poly oct1(200, 200, 50, 8, M_PI / 8);
	oct1.draw(img, Image::red, -1);

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