#include "body.hpp"
#include "lvl.hpp"
#include <cmath>

static double tillwhole(double, double);

void Body::move(const Lvl &lvl, double dx) {
	double xmul = dx > 0 ? 1 : -1;
	double ymul = dy > 0 ? 1 : -1;
	Geom2d::Pt v(dx, dy);
	Geom2d::Pt left(fabs(v.x), fabs(v.y));
	Isect fallis;

	while (left.x > 0.0 || left.y > 0.0) {
		Geom2d::Pt d(step(v));
		left.x -= fabs(d.x);
		left.y -= fabs(d.y);

		Isect is(lvl.isect(bbox, d));
		if (is.is && is.dy != 0.0)
			fallis = is;

		d.x = d.x - xmul * is.dx;
		d.y = d.y - ymul * is.dy;
		v.x -= d.x;
		v.y -= d.y;
		bbox.translate(d.x, d.y);
	}

	dofall(lvl, fallis);
}

Geom2d::Pt Body::step(const Geom2d::Pt &v) {
	Geom2d::Pt loc(bbox.min);
	Geom2d::Pt d(tillwhole(loc.x, v.x), tillwhole(loc.y, v.y));

	if (d.x == 0.0 && v.x != 0.0)
		d.x = v.x < 0 ? -1.0 : 1.0;

	if (fabs(d.x) > fabs(v.x))
		d.x = v.x;

	if (d.y == 0.0 && v.y != 0.0)
		d.y = v.y < 0 ? -1.0 : 1.0;

	if (fabs(d.y) > fabs(v.y))
		d.y = v.y;

	return d;
}

void Body::dofall(const Lvl &lvl, const Isect &is) {

	if(dy > 0 && is.dy > 0 && fall) { /* hit the ground */
		/* Constantly try to fall in order to test ground
		 * beneath us. */
		fall = false;
	} else if (dy < 0 && is.dy > 0) { /* hit my head on something */
		dy = 0;
		fall = true;
	}

	if (!is.is && !fall) { /* are we falling now? */
		dy = 0;
		fall = true;
	}

	if (fall && dy < Maxdy) {
		Lvl::Blkinfo bi = lvl.majorblk(bbox);
		dy += bi.tile.gravity();
	}
}

static double tillwhole(double loc, double vel)
{
	if (vel > 0)
		return ceil(loc) - loc;
	return floor(loc) - loc;
}