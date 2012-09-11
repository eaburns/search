#pragma once

#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include "../utils/geom2d.hpp"

struct Color {
	Color() : r(1), g(1), b(1) { }
	Color(double rvl, double gvl, double bvl) {
		setred(rvl);
		setgreen(gvl);
		setblue(bvl);
	}
	void setred(double d) { r = clamp(d); }
	void setgreen(double d) { g = clamp(d); }
	void setblue(double d) { b = clamp(d); }
	double getred() const { return r; }
	double getgreen() const { return g; }
	double getblue() const { return b; }
	unsigned char getred255() const { return 255 * r; }
	unsigned char getgreen255() const { return 255 * g; }
	unsigned char getblue255() const { return 255 * b; }
private:
	static double clamp(double d) {
		if (d < 0.0) return 0.0;
		if (d > 1.0) return 1.0;
		return d;
	}
	double r, g, b;
};

// An array of colors that can be used for
// conveniently cycling through some
// OK colors.
extern const Color somecolors[];
extern const unsigned int Nsomecolors;

struct Image {
	static const Color red, green, blue, white, black;

	Image(unsigned int w, unsigned int h, const char *t = "<untitled>") :
		width(w), height(h), title(t), pixels(NULL)
		{ }

	~Image();

	void saveeps(const char *, bool usletter = false, int marginpt = -1) const;

	void writeeps(FILE*, bool usletter = false, int marginpt = -1) const;

	struct Drawable {
		Drawable() { }
		virtual ~Drawable() { }
		virtual void writeeps(FILE*) const = 0;
	};

	struct Text : public Drawable {

		enum Position { Left, Right, Centered };

		Text(const char *txt, const geom2d::Pt &p) :
			loc(p), sz(12), pos(Centered), font("Times-Roman"),
			text(txt), c(black)
			{ }

		Text(const char *txt, double x, double y) :
			loc(x, y), sz(12), pos(Centered), font("Times-Roman"),
			text(txt), c(black)
			{ }

		virtual void writeeps(FILE*) const;

		geom2d::Pt loc;
		double sz;
		enum Position pos;
		std::string font, text;
		Color c;
	};

	struct Pt : public Drawable, public geom2d::Pt {

		Pt(const geom2d::Pt &p, const Color &color, double rad, double width) : 
			geom2d::Pt(p), r(rad), w(width), c(color)
			{ }

		virtual void writeeps(FILE*) const;

		double r, w;
		Color c;
	};

	struct Line : public Drawable, public geom2d::LineSg {

		Line(const geom2d::Pt &p0, const geom2d::Pt &p1,
				const Color &color, double width) :
			geom2d::LineSg(p0, p1), w(width), c(color)
			{ }

		Line(const geom2d::LineSg &l, const Color &color, double width) :
			geom2d::LineSg(l), w(width), c(color)
			{ }

		virtual void writeeps(FILE*) const;

		double w;
		Color c;
	};
	
	struct Arc : public Drawable, public geom2d::Arc {

		Arc(const geom2d::Arc &a, const Color &color, double width) :
			geom2d::Arc(a), w(width), c(color)
			{ }

		virtual void writeeps(FILE*) const;

		double w;
		Color c;
	};

	struct Poly : public Drawable, public geom2d::Poly  {

		Poly() {}

		Poly(const geom2d::Poly &p, const Color &color, double width) :
			geom2d::Poly(p), w(width), c(color)
			{ }

		virtual void writeeps(FILE*) const;

		double w;
		Color c;
	};

	void add(Drawable *d) { comps.push_back(d); }

	void setpixel(unsigned int x, unsigned int y, Color c) {
		if (!pixels)
			pixels = new Color[width * height];
		pixels[y * width + x] = c;
	}

	unsigned int width, height;
	std::string title;
	std::vector<Drawable*> comps;
	Color *pixels;

private:

	/* 72/2 pt == ½ in */
	void write_epshdrletter(FILE*, unsigned int marginpt = 72/2) const;
	void write_epshdr(FILE*, unsigned int marginpt = 0) const;
	void write_epsdata(FILE*) const;
	std::string encode_epsdata() const;
};
