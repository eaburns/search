#include "scene.hpp"
#include <GL/gl.h>

void Scene::render(void) const {
	for (unsigned int i = 0; i < comps.size(); i++)
		comps[i]->render();
}

void Scene::save(const char *file) const {
	Image img(width, height, file);
	for (unsigned int i = 0; i < comps.size(); i++)
		img.add(comps[i]);
	img.saveeps(file);
}

void Scene::Pt::render(void) const {
	glColor3d(c.getred(), c.getgreen(), c.getblue());
	if (w > 0)
		glLineWidth(w);
	glPointSize(Image::Pt::r);

	glBegin(GL_POINTS);
	glVertex2d(x, y);
	glEnd();
}

void Scene::Line::render(void) const {
	glColor3f(c.getred(), c.getgreen(), c.getblue());
	if (w > 0)
		glLineWidth(w);
	glBegin(GL_LINES);
	glVertex2d(p0.x, p0.y);
	glVertex2d(p1.x, p1.y);
	glEnd();
}

Scene::Arc::Arc(const Geom2d::Arc &a, const Color &c, double w) : Image::Arc(a, c, w) {
	double dt = (t1 - t0) / (Narcpts - 1);
	double t = t0;
	for (unsigned int i = 0; i < Narcpts; i++) {
		pts[i].x = Geom2d::Arc::c.x + cos(t) * r;
		pts[i].y = Geom2d::Arc::c.y + sin(t) * r;
		t += dt;
	}
	assert (pts[0] == start());
	assert (pts[Narcpts-1] == end());
}

void Scene::Arc::render(void) const {
	glColor3f(c.getred(), c.getgreen(), c.getblue());
	if (w > 0)
		glLineWidth(w);
	glBegin(GL_LINE_STRIP);
	for (unsigned int i = 0; i < Narcpts; i++)
		glVertex2d(pts[i].x, pts[i].y);
	glEnd();
}

void Scene::Poly::render(void) const {
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