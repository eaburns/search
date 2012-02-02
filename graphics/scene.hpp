#include "../utils/image.hpp"
#include <vector>

struct Scene {

	Scene(unsigned int w, unsigned int h) : width(w), height(h) { }

	void render(void) const;

	// save saves the scene to the given file.
	void save(const char*) const;

	struct Renderable : public Image::Drawable {
		virtual void render(void) const = 0;
	};

	void add(Renderable *v) { comps.push_back(v); }

	struct Point : public Renderable, public Image::Point {

		Point(const Geom2d::Point &p, const Color &c, double _r = 1, double w = 1) :
			Image::Point(p, c, r, w)
			{ }

		virtual void writeeps(FILE *out) const { Image::Point::writeeps(out); }

		virtual void render(void) const;

	private:
		double r;
	};

	struct Line : public Renderable, public Image::Line {

		Line(const Geom2d::LineSeg &l, const Color &c, double w = 1) :
			Image::Line(l, c, w) { }

		virtual void writeeps(FILE *out) const { Image::Line::writeeps(out); }

		virtual void render(void) const;
	};

	struct Arc : public Renderable, public Image::Arc {

		Arc(const Geom2d::Arc &a, const Color &c, double w = 1) :
			Image::Arc(a, c, w) { }

		virtual void writeeps(FILE *out) const { Image::Arc::writeeps(out); }

		virtual void render(void) const;
	};

	struct Polygon : public Renderable, public Image::Polygon {

		Polygon(const Geom2d::Polygon &p, const Color &c, double w = 1) :
			Image::Polygon(p, c, w) { }

		virtual void writeeps(FILE *out) const { Image::Polygon::writeeps(out); }

		virtual void render(void) const;
	};

	unsigned int width, height;
	std::vector<Renderable*> comps;
};