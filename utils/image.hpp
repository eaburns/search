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
		Drawable(const Color &_c) : c(_c) { }
		virtual void writeeps(FILE*) const;
		virtual const char *name(void) const = 0;
		Color c;
	};

	struct Text : public Drawable {

		enum Position { Left, Right, Centered };

		Text(const char *_text, const Geom2d::Point &p) :
			Drawable(black), loc(p), sz(12), pos(Centered),
			font("Times-Roman"), text(_text)
			{ }

		Text(const char *_text, double x, double y) :
			Drawable(black), loc(x, y), sz(12), pos(Centered),
			font("Times-Roman"), text(_text)
			{ }

		virtual void writeeps(FILE*) const;

		virtual const char *name(void)const { return "Text"; }

		Geom2d::Point loc;
		double sz;
		enum Position pos;
		std::string font, text;
	};

	struct Point : public Drawable, public Geom2d::Point {

		// Point constructs a new point at the given location with
		// the given color and radius.  If the radius is positive then
		// the point filled and if the radius is non-positive then the
		// point is not outlined an the radius is used as the absolute
		// value of the specified radius.
		Point(const Geom2d::Point &p, const Color &c, double _r) : 
			Drawable(c), Geom2d::Point(p), r(_r)
			{ }

		virtual void writeeps(FILE*) const;

		virtual const char *name(void) const { return "Point"; }

		double r;
	};

	struct Line : public Drawable, public Geom2d::LineSeg {

		Line(const Geom2d::Point &p0, const Geom2d::Point &p1,
				const Color &c, double _w) :
			Drawable(c), Geom2d::LineSeg(p0, p1), w(_w)
			{ }

		Line(const Geom2d::LineSeg &l, const Color &c, double _w) :
			Drawable(c), Geom2d::LineSeg(l), w(_w)
			{ }

		virtual void writeeps(FILE*) const;

		virtual const char *name(void) const { return "Line"; }

		double w;
	};
	
	struct Arc : public Drawable, public Geom2d::Arc {

		Arc(const Geom2d::Arc &a, const Color &c, double _w) :
			Drawable(c), Geom2d::Arc(a), w(_w)
			{ }

		virtual void writeeps(FILE*) const;

		virtual const char *name(void) const { return "Arc"; }

		double w;
	};

	struct Polygon : public Drawable, public Geom2d::Polygon  {

		Polygon(const Geom2d::Polygon &p, const Color &c, double _w) :
			Drawable(c), Geom2d::Polygon(p), w(_w)
			{ }

		virtual void writeeps(FILE*) const;

		virtual const char *name(void) const { return "Polygon"; }

		double w;
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
