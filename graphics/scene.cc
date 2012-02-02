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

void Scene::Point::render(void) const {
	glColor3d(c.getred(), c.getgreen(), c.getblue());
	if (w > 0)
		glLineWidth(w);
	glPointSize(Image::Point::r);

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

void Scene::Arc::render(void) const {
}

void Scene::Polygon::render(void) const {
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