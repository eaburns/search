#include "../utils/image.hpp"
#include <vector>

struct Scene {

	Scene(unsigned int w, unsigned int h) : width(w), height(h) { }

	void render(void) const;

	// clear clears the components of the scene.
	void clear(void) {
		while (!comps.empty()) {
			Renderable *r = comps.back();
			comps.pop_back();
			delete r;
		}
	}

	// save saves the scene to the given file.
	void save(const char *file) const;

	struct Renderable : public Image::Drawable {
		virtual void render(void) const = 0;
	};

	void add(Renderable *v) { comps.push_back(v); }

	struct Pt : public Renderable, public Image::Pt {

		Pt(const geom2d::Pt &p, const Color &c, double r = 1, double w = 1) :
			Image::Pt(p, c, r, w)
			{ }

		virtual void writeeps(FILE *out) const { Image::Pt::writeeps(out); }

		virtual void render(void) const;
	};

	struct Line : public Renderable, public Image::Line {

		Line(const geom2d::LineSg &l, const Color &c, double w = 1) :
			Image::Line(l, c, w) { }

		virtual void writeeps(FILE *out) const { Image::Line::writeeps(out); }

		virtual void render(void) const;
	};

	struct Arc : public Renderable, public Image::Arc {

		Arc(const geom2d::Arc &a, const Color &c, double w = 1);

		virtual void writeeps(FILE *out) const { Image::Arc::writeeps(out); }

		virtual void render(void) const;
	private:
		enum { Narcpts = 10 };

		geom2d::Pt pts[Narcpts];
	};

	struct Poly : public Renderable, public Image::Poly {

		Poly(const geom2d::Poly &p, const Color &c, double w = 1) :
			Image::Poly(p, c, w) { }

		virtual void writeeps(FILE *out) const { Image::Poly::writeeps(out); }

		virtual void render(void) const;
	};

	unsigned int width, height;
	std::vector<Renderable*> comps;
};