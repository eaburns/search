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

const Color somecolors[] = {
	Color(0.0, 0.6, 0.0),	// Dark green
	Color(0.0, 0.0, 0.6),	// Dark blue
	Color(0.5, 0.0, 0.5),	// Purple
	Color(1.0, 0.54, 0.0),	// Dark orange
	Color(0.28, 0.24, 0.55),	// Slate blue
	Color(0.42, 0.56, 0.14),	// Olive drab
	Color(0.25, 0.80, 0.80),	// "Skyish"
	Color(0.80, 0.80, 0.20),	// Mustard
};

const unsigned int Nsomecolors = sizeof(somecolors) / sizeof(somecolors[0]);

Image::Image(unsigned int width, unsigned int height, const char *t) :
		w(width), h(height), title(t) {
	data = new Color[w * h];
}

Image::~Image(void) {
	while (!comps.empty()) {
		delete comps.back();
		comps.pop_back();
	}
	delete[] data;
}

void Image::save(const char *path, bool usletter, int marginpt) const {
	FILE *f = fopen(path, "w");
	if (!f)
		fatalx(errno, "Failed to open %s for writing\n", path);
	output(f, usletter, marginpt);
	fclose(f);
}

void Image::output(FILE *out, bool usletter, int marginpt) const {
	if (marginpt < 0)
		marginpt = usletter ? 72/2 : 0;	/* 72/2 pt == ½ in */

	if (usletter)
		outputhdr_usletter(out, marginpt);
	else
		outputhdr(out, marginpt);

	outputdata(out);

	fprintf(out, "1 setlinejoin\n");	// Round join

	for (unsigned int i = 0; i < comps.size(); i++) {
		fputc('\n', out);
		comps[i]->write(out);
	}
	fprintf(out, "showpage\n");
}

enum {
	Widthpt = 612,	/* pts == 8.5 in */
	Heightpt = 792,	/* pts = 11 in */
};

void Image::outputhdr_usletter(FILE *out, unsigned int marginpt) const {
	fprintf(out, "%%!PS-Adobe-3.0\n");
	fprintf(out, "%%%%Creator: UNH-AI C++ Search Framework\n");
	fprintf(out, "%%%%Title: %s\n", title.c_str());
	fprintf(out, "%%%%BoundingBox: 0 0 %u %u\n", Widthpt, Heightpt);
	fprintf(out, "%%%%EndComments\n");

	double maxw = Widthpt - marginpt * 2, maxh = Heightpt - marginpt * 2;
	double scalex = maxw / w, scaley = maxh / h;
	double transx = marginpt, transy = (Heightpt - h * scalex) / 2;

	double scale = scalex;
	if (scaley < scalex) {
		scale = scaley;
		transx = (Widthpt - w * scaley) / 2;
	}

	fprintf(out, "%g %g translate\n", transx, transy);
	fprintf(out, "%g %g scale\n", scale, scale);
}

void Image::outputhdr(FILE *out, unsigned int marginpt) const {
	fprintf(out, "%%!PS-Adobe-3.0\n");
	fprintf(out, "%%%%Creator: UNH-AI C++ Search Framework\n");
	fprintf(out, "%%%%Title: %s\n", title.c_str());
	fprintf(out, "%%%%BoundingBox: 0 0 %u %u\n", w + 2 * marginpt, h + 2 * marginpt);
	fprintf(out, "%%%%EndComments\n");
	fprintf(out, "%u %u translate\n", marginpt, marginpt);
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
	if (_closepath)
		fprintf(out, "closepath\n");
	if (!_fill)
		fprintf(out, "stroke\n");
	else
		fprintf(out, "fill\n");
}

void Image::Path::MoveTo::write(FILE *out) const {
	fprintf(out, "%g %g moveto\n", x, y);
}

Image::Path::Loc Image::Path::MoveTo::move(Loc p) const {
	return Loc(std::pair<double,double>(x, y));
}

void Image::Path::LineTo::write(FILE *out) const {
	fprintf(out, "%g %g lineto\n", x, y);
}

Image::Path::Loc Image::Path::LineTo::move(Loc p) const {
	return Loc(std::pair<double,double>(x, y));
}

void Image::Path::SetLineWidth::write(FILE *out) const {
	fprintf(out, "%g setlinewidth\n", w);
}

Image::Path::Loc Image::Path::SetLineWidth::move(Loc p) const {
	return p;
}

void Image::Path::SetColor::write(FILE *out) const {
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(),
		c.getgreen(), c.getblue());
}

Image::Path::Loc Image::Path::SetColor::move(Loc p) const {
	return p;
}

void Image::Path::Arc::write(FILE *out) const {
	const char *fun = "arc";
	if (dt < 0)
		fun = "arcn";
	fprintf(out, "%g %g %g %g %g %s\n", x, y, r, t, t + dt, fun);
}

Image::Path::Loc Image::Path::Arc::move(Loc p) const {
	double x1 = x + r * cos((t + dt) * M_PI / 180);
	double y1 = y + r * sin((t + dt) * M_PI / 180);
	return Loc(std::pair<double,double>(x1, y1));
}

static bool dbleq(double a, double b) {
	static const double Epsilon = 0.01;
	return fabs(a - b) < Epsilon;
}

void Image::Path::line(double x0, double y0, double x1, double y1) {
	if (!cur || !dbleq(cur->first, x0) || !dbleq(cur->second, y0))
		addseg(new MoveTo(x0, y0));
	addseg(new LineTo(x1, y1));
}

void Image::Path::curve(double xc, double yc, double r, double t, double dt) {
	NauticalArc *a = new NauticalArc(xc, yc, r, t, dt);

	double x0 = xc + a->r * cos(a->t * M_PI / 180);
	double y0 = yc + a->r * sin(a->t * M_PI / 180);

	if (!cur || !dbleq(cur->first, x0) || !dbleq(cur->second, y0))
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
	if (linewidth >= 0)
		fprintf(out, "%g setlinewidth\n", linewidth);
	else
		fprintf(out, "0.1 setlinewidth\n");
	fprintf(out, "%g %g moveto\n", x0r + x, y0r + y);
	fprintf(out, "%g %g lineto\n", x1r + x, y1r + y);
	fprintf(out, "%g %g lineto\n", x2r + x, y2r + y);
	fprintf(out, "closepath\n");
	if (linewidth < 0)
		fprintf(out, "fill\n");
	else
		fprintf(out, "stroke\n");
}

void Image::Circle::write(FILE *out) const {
	fprintf(out, "%% Circle\n");
	const char *finish = "stroke";
	if (lwidth <= 0) {
		finish = "fill";
		fprintf(out, "0.1 setlinewidth\n");
	} else {
		fprintf(out, "%g setlinewidth\n", lwidth);
	}
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	fprintf(out, "newpath %g %g %g 0 360 arc %s\n", x, y, r, finish);
}
