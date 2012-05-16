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

Image::~Image() {
	while (!comps.empty()) {
		delete comps.back();
		comps.pop_back();
	}
	if (pixels)
		delete[] pixels;
}

void Image::saveeps(const char *path, bool usletter, int marginpt) const {
	FILE *f = fopen(path, "w");
	if (!f)
		fatalx(errno, "Failed to open %s for writing\n", path);
	writeeps(f, usletter, marginpt);
	fclose(f);
}

void Image::writeeps(FILE *out, bool usletter, int marginpt) const {
	if (marginpt < 0)
		marginpt = usletter ? 72/2 : 0;	/* 72/2 pt == ½ in */

	if (usletter)
		write_epshdrletter(out, marginpt);
	else
		write_epshdr(out, marginpt);

	if (pixels)
		write_epsdata(out);

	for (unsigned int i = 0; i < comps.size(); i++) {
		fputc('\n', out);
		comps[i]->writeeps(out);
	}
	fprintf(out, "showpage\n");
}

enum {
	Widthpt = 612,	/* pts == 8.5 in */
	Heightpt = 792,	/* pts = 11 in */
};

void Image::write_epshdrletter(FILE *out, unsigned int marginpt) const {
	fprintf(out, "%%!PS-Adobe-3.0\n");
	fprintf(out, "%%%%Creator: UNH-AI C++ Search Framework\n");
	fprintf(out, "%%%%Title: %s\n", title.c_str());
	fprintf(out, "%%%%BoundingBox: 0 0 %u %u\n", Widthpt, Heightpt);
	fprintf(out, "%%%%EndComments\n");

	double maxw = Widthpt - marginpt * 2, maxh = Heightpt - marginpt * 2;
	double scalex = maxw / width, scaley = maxh / height;
	double transx = marginpt, transy = (Heightpt - height * scalex) / 2;

	double scale = scalex;
	if (scaley < scalex) {
		scale = scaley;
		transy = marginpt;
		transx = (Widthpt - width * scaley) / 2;
	}

	fprintf(out, "%g %g translate\n", transx, transy);
	fprintf(out, "%g %g scale\n", scale, scale);
}

void Image::write_epshdr(FILE *out, unsigned int marginpt) const {
	fputs("%%!PS-Adobe-3.0\n", out);
	fputs("%%%%Creator: UNH-AI C++ Search Framework\n", out);
	fprintf(out, "%%%%Title: %s\n", title.c_str());
	fprintf(out, "%%%%BoundingBox: 0 0 %u %u\n",
		width + 2 * marginpt, height + 2 * marginpt);
	fputs("%%%%EndComments\n", out);
	fprintf(out, "%u %u translate\n", marginpt, marginpt);
}

void Image::write_epsdata(FILE *out) const {
	fputs("\n%% Image data\n", out);
	fputs("gsave\n", out);
	fprintf(out, "%u %u scale	%% scale pixels to points\n", width, height);
	fprintf(out, "%u %u 8 [%u 0 0 %u 0 0]	", width, height, width, height);
	fputs("%% width height colordepth transform\n", out);
	fputs("/datasource currentfile ", out);
	fputs("/ASCII85Decode filter /RunLengthDecode filter def\n", out);
	fprintf(out, "/datastring %u string def	", width * 3);
	fprintf(out, "%% %u = width * color components\n", width * 3);
	fputs("{datasource datastring readstring pop}\n", out);
	fputs("false	%% false == single data source (rgb)\n", out);
	fputs("3	%% number of color components\n", out);
	fputs("colorimage\n", out);
	fprintf(out, "%s\n", encode_epsdata().c_str());
	fputs("grestore\n", out);
}

std::string Image::encode_epsdata() const {
	std::string cs;
	for (unsigned int i = 0; i < width * height; i++) {
		cs.push_back(pixels[i].getred255());
		cs.push_back(pixels[i].getgreen255());
		cs.push_back(pixels[i].getblue255());
	}
	std::string rlenc = runlenenc(cs);
	std::string dst = ascii85enc(rlenc);
	dst.push_back('~');
	dst.push_back('>');
	return dst;
}

void Image::Text::writeeps(FILE *out) const {
	fputs("% Text\n", out);
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	fprintf(out, "/%s findfont %g scalefont setfont\n", font.c_str(), sz);
	fprintf(out, "%g %g moveto\n", loc.x, loc.y);
	fprintf(out, "(%s) ", text.c_str());
	switch (pos) {
	case Left:
		fputs("show\n", out);
		break;
	case Right:
		fputs("dup stringwidth pop neg 0 rmoveto show\n", out);
		break;
	case Centered:
		fputs("dup stringwidth pop 2 div neg 0 rmoveto show\n", out);
		break;
	default:
		fatal("Unknown text position: %d\n", pos);
	}
}

void Image::Pt::writeeps(FILE *out) const {
	fputs("% Pt\n", out);
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	const char *finish = "stroke\n";
	if (w >= 0) {
		finish = "fill\n";
		fputs("0.1 setlinewidth\n", out);
	} else {
		fprintf(out, "%g setlinewidth\n", w);
	}
	fprintf(out, "newpath %g %g %g 0 360 arc\n", x, y, r);
	fputs(finish, out);
}

void Image::Line::writeeps(FILE *out) const {
	fputs("% Line\n", out);
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	fprintf(out, "%g setlinewidth\n", w);
	fputs("newpath\n", out);
	fprintf(out, "%g %g moveto\n", p0.x, p0.y);
	fprintf(out, "%g %g lineto\n", p1.x, p1.y);
	fputs("stroke\n", out);
}

void Image::Arc::writeeps(FILE *out) const {
	fputs("% Arc\n", out);
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	fprintf(out, "%g setlinewidth\n", w);
	fputs("newpath\n", out);
	geom2d::Pt p(start());
	fprintf(out, "%g %g moveto\n", p.x, p.y);
	double d0 = t0 * (180 / M_PI), d1 = t1 * (180 / M_PI);
	fprintf(out, "%g %g %g %g %g arc\n",
		geom2d::Arc::c.x, geom2d::Arc::c.y, r, d0, d1);
	fputs("stroke\n", out);
}

void Image::Poly::writeeps(FILE *out) const {
	fputs("% Poly\n", out);
	fprintf(out, "%g %g %g setrgbcolor\n", c.getred(), c.getgreen(), c.getblue());
	const char *finish = "stroke\n";
	if (w <= 0) {
		finish = "fill\n";
		fputs("0.1 setlinewidth\n", out);
	} else {
		fprintf(out, "%g setlinewidth\n", w);
	}
	fprintf(out, "%% %g",w);
	fputs("newpath\n", out);
	fprintf(out, "%g %g moveto\n", verts[0].x, verts[0].y);
	for (unsigned int i = 1; i < verts.size(); i++)
		fprintf(out, "%g %g lineto\n", verts[i].x, verts[i].y);
	fputs("closepath\n", out);
	fputs(finish, out);
}