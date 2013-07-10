// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "scene.hpp"
#include "ui.hpp"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

void Scene::render() const {
	for (unsigned int i = 0; i < comps.size(); i++)
		comps[i]->render();
}

void Scene::save(const char *file) const {
	Image img(width, height, file);
	for (unsigned int i = 0; i < comps.size(); i++)
		img.add(comps[i]);
	img.saveeps(file);
}

void Scene::Pt::render() const {
	glColor3d(c.getred(), c.getgreen(), c.getblue());
	if (w > 0)
		glLineWidth(w);
	glPointSize(Image::Pt::r);

	glBegin(GL_POINTS);
	glVertex2d(x, y);
	glEnd();
}

void Scene::Line::render() const {
	glColor3f(c.getred(), c.getgreen(), c.getblue());
	if (w > 0)
		glLineWidth(w);
	if(pattern != SOLID) {
		glEnable(GL_LINE_STIPPLE);
		if(pattern == DASHED)
			glLineStipple(1, 0x00FF);
		else
			glLineStipple(1, 0x0101);
	}
	glBegin(GL_LINES);
	glVertex2d(p0.x, p0.y);
	glVertex2d(p1.x, p1.y);
	glEnd();
	if(pattern != SOLID)
		glDisable(GL_LINE_STIPPLE);
}

Scene::Arc::Arc(const geom2d::Arc &a, const Color &c, double w) : Image::Arc(a, c, w) {
	double step = dt / (Narcpts - 1);
	double t = t0;
	for (unsigned int i = 0; i < Narcpts; i++) {
		pts[i].x = geom2d::Arc::c.x + cos(t) * r;
		pts[i].y = geom2d::Arc::c.y + sin(t) * r;
		t += step;
	}
	assert (pts[0] == start());
	assert (pts[Narcpts-1] == end());
}

void Scene::Arc::render() const {
	glColor3f(c.getred(), c.getgreen(), c.getblue());
	if (w > 0)
		glLineWidth(w);
	glBegin(GL_LINE_STRIP);
	for (unsigned int i = 0; i < Narcpts; i++)
		glVertex2d(pts[i].x, pts[i].y);
	glEnd();
}

void Scene::Poly::render() const {
	glColor3d(c.getred(), c.getgreen(), c.getblue());
	if (w > 0) {
		glLineWidth(w);
		glBegin(GL_LINE_STRIP);
	} else {
		glPolygonMode(GL_FRONT, GL_FILL);
		glBegin(GL_POLYGON);
	}
	for (unsigned int i = 0; i < verts.size(); i++)
		glVertex2d(verts[i].x, verts[i].y);
	glVertex2d(verts[0].x, verts[0].y);
	glEnd();
}

Scene::Img::Img(const std::string &path) : smin(0), tmin(0), smax(1), tmax(1) {
	SDL_Surface *surf = IMG_Load(path.c_str());
	if (!surf)
		fatal("Failed to load image %s: %s", path.c_str(), IMG_GetError());

	w = texw = surf->w;
	h = texh = surf->h;

	GLint pxSz = surf->format->BytesPerPixel;
	GLenum texFormat = GL_BGRA;
	switch (pxSz) {
	case 4:
		if (surf->format->Rmask == 0xFF)
			texFormat = GL_RGBA;
		break;
	case 3:
		if (surf->format->Rmask == 0xFF)
			texFormat = GL_RGB;
		else
			texFormat = GL_BGR;
		break;
	default:
		fatal("Bad image color type… apparently");
	}

	Ui::chkerror("before gen texture");

	glGenTextures(1, &texid);
	Ui::chkerror("after gen texture");

	glBindTexture(GL_TEXTURE_2D, texid);
	Ui::chkerror("after bind texture");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	Ui::chkerror("after parameteri 1");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	Ui::chkerror("after parameteri 2");

	glTexImage2D(GL_TEXTURE_2D, 0, pxSz, surf->w, surf->h, 0,
		texFormat, GL_UNSIGNED_BYTE, surf->pixels);
	Ui::chkerror("after tex image 2d");

	SDL_FreeSurface(surf);
}

void Scene::Img::render() const {
	glColor3d(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texid);

	glBegin(GL_QUADS);

	glTexCoord2d(smin, tmin);
	glVertex3d(x, y, 0);

	glTexCoord2d(smin, tmax);
	glVertex3d(x, y+h, 0);

	glTexCoord2d(smax, tmax);
	glVertex3d(x+w, y+h, 0);

	glTexCoord2d(smax, tmin);
	glVertex3d(x+w, y, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}
