#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include "geom2d.hpp"

struct Color {
	Color(void) : r(1), g(1), b(1) { }
	Color(double _r, double _g, double _b) {
		setred(_r);
		setgreen(_g);
		setblue(_b);
	}
	void setred(double d) { r = clamp(d); }
	void setgreen(double d) { g = clamp(d); }
	void setblue(double d) { b = clamp(d); }
	double getred(void) const { return r; }
	double getgreen(void) const { return g; }
	double getblue(void) const { return b; }
	unsigned char getred255(void) const { return 255 * r; }
	unsigned char getgreen255(void) const { return 255 * g; }
	unsigned char getblue255(void) const { return 255 * b; }
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

	~Image(void);

	void saveeps(const char *, bool usletter = false, int marginpt = -1) const;

	void writeeps(FILE*, bool usletter = false, int marginpt = -1) const;

	struct Drawable {
		Drawable(void) { }
		virtual void writeeps(FILE*) const = 0;
	};

	struct Text : public Drawable {

		enum Position { Left, Right, Centered };

		Text(const char *_text, const Geom2d::Pt &p) :
			loc(p), sz(12), pos(Centered), font("Times-Roman"),
			text(_text), c(black)
			{ }

		Text(const char *_text, double x, double y) :
			loc(x, y), sz(12), pos(Centered), font("Times-Roman"),
			text(_text), c(black)
			{ }

		virtual void writeeps(FILE*) const;

		Geom2d::Pt loc;
		double sz;
		enum Position pos;
		std::string font, text;
		Color c;
	};

	struct Pt : public Drawable, public Geom2d::Pt {

		Pt(const Geom2d::Pt &p, const Color &_c, double _r, double _w) : 
			Geom2d::Pt(p), r(_r), w(_w), c(_c)
			{ }

		virtual void writeeps(FILE*) const;

		double r, w;
		Color c;
	};

	struct Line : public Drawable, public Geom2d::LineSg {

		Line(const Geom2d::Pt &p0, const Geom2d::Pt &p1,
				const Color &_c, double _w) :
			Geom2d::LineSg(p0, p1), w(_w), c(_c)
			{ }

		Line(const Geom2d::LineSg &l, const Color &_c, double _w) :
			Geom2d::LineSg(l), w(_w), c(_c)
			{ }

		virtual void writeeps(FILE*) const;

		double w;
		Color c;
	};
	
	struct Arc : public Drawable, public Geom2d::Arc {

		Arc(const Geom2d::Arc &a, const Color &_c, double _w) :
			Geom2d::Arc(a), w(_w), c(_c)
			{ }

		virtual void writeeps(FILE*) const;

		double w;
		Color c;
	};

	struct Poly : public Drawable, public Geom2d::Poly  {

		Poly(const Geom2d::Poly &p, const Color &_c, double _w) :
			Geom2d::Poly(p), w(_w), c(_c)
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
	std::string encode_epsdata(void) const;
};

#endif	// _IMAGE_HPP_
