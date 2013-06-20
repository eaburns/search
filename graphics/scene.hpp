// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#pragma once
#include "../graphics/image.hpp"
#include <SDL/SDL_opengl.h>
#include <vector>
#include <string>

void fatal(const char*, ...);

struct Scene {

	Scene(unsigned int w, unsigned int h) : width(w), height(h) { }

	void render() const;

	// clear clears the components of the scene.
	void clear() {
		while (!comps.empty()) {
			Renderable *r = comps.back();
			comps.pop_back();
			delete r;
		}
	}

	// save saves the scene to the given file.
	void save(const char *file) const;

	struct Renderable : public Image::Drawable {
		virtual void render() const = 0;
	};

	void add(Renderable *v) { comps.push_back(v); }

	struct Pt : public Renderable, public Image::Pt {

		Pt(const geom2d::Pt &p, const Color &c, double r = 1, double w = 1) :
			Image::Pt(p, c, r, w)
			{ }

		virtual void writeeps(FILE *out) const { Image::Pt::writeeps(out); }

		virtual void render() const;
	};

	struct Line : public Renderable, public Image::Line {
		enum Pattern { SOLID, DASHED, DOTTED } pattern;

		Line(const geom2d::LineSg &l, const Color &c, double w = 1, enum Pattern p = SOLID) :
			Image::Line(l, c, w), pattern(p) { }

		virtual void writeeps(FILE *out) const { Image::Line::writeeps(out); }

		virtual void render() const;

	};

	struct Arc : public Renderable, public Image::Arc {

		Arc(const geom2d::Arc &a, const Color &c, double w = 1);

		virtual void writeeps(FILE *out) const { Image::Arc::writeeps(out); }

		virtual void render() const;
	private:
		enum { Narcpts = 10 };

		geom2d::Pt pts[Narcpts];
	};

	struct Poly : public Renderable, public Image::Poly {

		Poly(const geom2d::Poly &p, const Color &c, double w = 1) :
			Image::Poly(p, c, w) { }

		virtual void writeeps(FILE *out) const { Image::Poly::writeeps(out); }

		virtual void render() const;
	};

	struct Img : public Renderable {
		Img() { }

		// This constructor loads an image from a file.  A typical use
		// case will be to load an image from that is never added to a scene,
		// and thus never destructed.  Instead, copies of the image can be
		// added to the scene.
		Img(const std::string&);

		Img(const Img &o) : smin(o.smin), tmin(o.tmin),
			smax(o.smax), tmax(o.tmax), x(o.x), y(o.y),
			w(o.w), h(o.h), texw(o.texw), texh(o.texh), texid(o.texid) { }

		// This constructor copies the image but moves it to
		// a different x,y location.
		Img(const Img &o, double x, double y) : smin(o.smin), tmin(o.tmin),
			smax(o.smax), tmax(o.tmax), x(x), y(y),
			w(o.w), h(o.h), texw(o.texw), texh(o.texh), texid(o.texid) { }

		virtual void writeeps(FILE*) const {
			fatal("Writing images to .eps is not supported");
		}

		virtual void render() const;

		// These are the texture coordinates used to draw
		// the image.
		double smin, tmin, smax, tmax;

		// x, y, w, and h define the image's rectangle.
		double x, y, w, h;

		// texw and texh are the width and height of the texture.
		unsigned int texw, texh;

	private:
		GLuint texid;
	};

	unsigned int width, height;
	std::vector<Renderable*> comps;
};
