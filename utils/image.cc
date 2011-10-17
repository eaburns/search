#include "utils.hpp"
#include "image.hpp"
#include <cerrno>
#include <cstdio>
#include <cassert>

const Color Image::red(1, 0, 0);
const Color Image::green(0, 1, 0);
const Color Image::blue(0, 0, 1);
const Color Image::black(0, 0, 0);
const Color Image::white(1, 1, 1);

Image::Image(const char *t, unsigned int width, unsigned int height) :
		w(width), h(height), title(t) {
	data = new Color[w * h];
}

Image::~Image(void) {
	while (!comps.empty()) {
		delete comps.back();
		comps.pop_back();
	}
	delete data;
}

void Image::save(const char *path) const {
	FILE *f = fopen(path, "w");
	if (!f)
		fatalx(errno, "Failed to open %s for writing\n", path);
	output(f);
	fclose(f);
}

void Image::output(FILE *out) const {
	outputhdr(out);
	outputdata(out);
	for (unsigned int i = 0; i < comps.size(); i++) {
		fputc('\n', out);
		comps[i]->write(out);
	}
	fprintf(out, "showpage\n");
}

void Image::outputhdr(FILE *out) const {
	fprintf(out, "%%!PS-Adobe-3.0\n");
	fprintf(out, "%%%%Creator: UNH-AI C++ Search Framework\n");
	fprintf(out, "%%%%Title: %s\n", title.c_str());
	fprintf(out, "%%%%BoundingBox: 0 0 %u %u\n", w, h);
	fprintf(out, "%%%%EndComments\n");
}

void Image::outputdata(FILE *out) const {
	fprintf(out, "\n%% Image data\n");
	fprintf(out, "gsave\n");
	fprintf(out, "%u %u scale	%% scale pixels to points\n", w, h);
	fprintf(out, "%u %u 8 [%u 0 0 %u 0 0]	", w, h, w, h);
	fprintf(out, "%% width height colordepth transform\n");
	fprintf(out, "/datasource currentfile ");
	fprintf(out, "/ASCII85Decode filter /RunLengthDecode filter def\n");
	fprintf(out, "/datastring %u string def	", w * 3);
	fprintf(out, "%% %u = width * color components\n", w * 3);
	fprintf(out, "{datasource datastring readstring pop}\n");
	fprintf(out, "false	%% false == single data source (rgb)\n");
	fprintf(out, "3	%% number of color components\n");
	fprintf(out, "colorimage\n");
	std::string encoded;
	encodedata(encoded);
	fprintf(out, "%s\n", encoded.c_str());
	fprintf(out, "grestore\n");
}

void Image::encodedata(std::string &dst) const {
	std::string cs;
	for (unsigned int i = 0; i < w * h; i++) {
		cs.push_back(data[i].getred255());
		cs.push_back(data[i].getgreen255());
		cs.push_back(data[i].getblue255());
	}
	std::string rlenc;
	runlenenc(rlenc, cs);
	ascii85enc(dst, rlenc);
	dst.push_back('~');
	dst.push_back('>');
}

Image::Path::~Path(void) {
	while (!segs.empty()) {
		delete segs.back();
		segs.pop_back();
	}
}

void Image::Path::write(FILE *out) const {
	fprintf(out, "%% Path\n");
	fprintf(out, "newpath\n");
	for (unsigned int i = 0; i < segs.size(); i++)
		segs[i]->write(out);
	fprintf(out, "stroke\n");
}

void Image::Path::MoveTo::write(FILE *out) const {
	fprintf(out, "%g %g moveto\n", x, y);
}

Image::Path::CurLoc Image::Path::MoveTo::move(CurLoc p) const {
	return CurLoc(Point(x, y));
}

void Image::Path::LineTo::write(FILE *out) const {
	fprintf(out, "%g %g lineto\n", x, y);
}

Image::Path::CurLoc Image::Path::LineTo::move(CurLoc p) const {
	return CurLoc(Point(x, y));
}

void Image::Path::SetLineWidth::write(FILE *out) const {
	fprintf(out, "%g setlinewidth\n", w);
}

Image::Path::CurLoc Image::Path::SetLineWidth::move(CurLoc p) const {
	return p;
}

void Image::Path::SetColor::write(FILE *out) const {
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(),
		c.getgreen(), c.getblue());
}

Image::Path::CurLoc Image::Path::SetColor::move(CurLoc p) const {
	return p;
}

void Image::Path::Arc::write(FILE *out) const {
	const char *fun = "arc";
	if (dt < 0)
		fun = "arcn";
	double t1 = fmod(t + dt, 360);
	fprintf(out, "%g %g %g %g %g %s\n", x, y, r, fmod(t, 360), t1, fun);
}

Image::Path::CurLoc Image::Path::Arc::move(CurLoc p) const {
	double t1 = t + dt;
	if (dt < 0)
		t1 = t - dt;
	return CurLoc(Point(x + r * cos(t1 * M_PI / 180), y + r * sin(t1* M_PI / 180)));
}

void Image::Path::line(double x0, double y0, double x1, double y1) {
	if (!endloc || endloc->x != x0 || endloc->y != y0)
		addseg(new MoveTo(x0, y0));
	addseg(new LineTo(x1, y1));
}

void Image::Path::curve(double xc, double yc, double r, double t, double dt) {
	NauticalArc *a = new NauticalArc(xc, yc, r, t, dt);

	double x0 = xc + a->r * cos(a->t * M_PI / 180);
	double y0 = yc + a->r * sin(a->t * M_PI / 180);

	if (!endloc || endloc->x != x0 || endloc->y != y0)
		addseg(new MoveTo(x0, y0));
	addseg(a);
}

void Image::Text::write(FILE *out) const {
	fprintf(out, "%% Text\n");
	fprintf(out, "/%s findfont %g scalefont setfont\n", font.c_str(), sz);
	fprintf(out, "%u %u moveto\n", x, y);
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	fprintf(out, "(%s) ", text.c_str());
	switch (pos) {
	case Left:
		fprintf(out, "show\n");
		break;
	case Right:
		fprintf(out, "dup stringwidth pop neg 0 rmoveto show\n");
		break;
	case Centered:
		fprintf(out, "dup stringwidth pop 2 div neg 0 rmoveto show\n");
		break;
	default:
		fatal("Unknown text position: %d\n", pos);
	}
}

void Image::Triangle::write(FILE *out) const {
	double wrad = w * M_PI / 180;
	double rotrad = rot * M_PI / 180;
	double side = ht / cos(wrad / 2);
	double xtop = ht / 2;
	double xbot = -ht / 2;
	double y1 = side * sin(wrad / 2);
	double y2 = -side * sin(wrad / 2);

	double x0r = xtop * cos(rotrad);
	double y0r = xtop * sin(rotrad);
	double x1r = xbot * cos(rotrad) - y1 * sin(rotrad);
	double y1r = xbot * sin(rotrad) + y1 * cos(rotrad);
	double x2r = xbot * cos(rotrad) - y2 * sin(rotrad);
	double y2r = xbot * sin(rotrad) + y2 * cos(rotrad);

	fprintf(out, "%% Triangle\n");
	fprintf(out, "newpath\n");
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	fprintf(out, "1 setlinewidth\n");
	fprintf(out, "%g %g moveto\n", x0r + x, y0r + y);
	fprintf(out, "%g %g lineto\n", x1r + x, y1r + y);
	fprintf(out, "%g %g lineto\n", x2r + x, y2r + y);
	fprintf(out, "closepath\n");
	if (fill)
		fprintf(out, "fill\n");
	else
		fprintf(out, "stroke\n");
}

void Image::Circle::write(FILE *out) const {
	fprintf(out, "%% Circle\n");
	const char *finish = "stroke";
	if (fill)
		finish = "fill";
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	fprintf(out, "newpath %g %g %g 0 360 arc %s\n", x, y, r, finish);
}