#include "geom.hpp"
#include "lvl.hpp"
#include <cmath>

static double tillwhole(double, double);

void Body::move(const Lvl &lvl) {
	double xmul = vel.x > 0 ? 1.0 : -1.0;
	double ymul = vel.y > 0 ? 1.0 : -1.0;
	Point v(vel);
	Point left(fabs(v.x), fabs(v.y));
	Isect fallis;

	while (left.x > 0.0 || left.y > 0.0) {
		Point d(step(v));
		left.x -= fabs(d.x);
		left.y -= fabs(d.y);

		Isect is(lvl.isection(z, bbox, d));
		printf("%g, %g → %g, %g\n", bbox.a.x, bbox.a.y, bbox.b.x, bbox.b.y);
		if (is.is && is.dy != 0.0) {
			printf("yis=%g, dy=%g\n", is.dy, d.y);
			fallis = is;
		} else {
			printf("no intersection\n");
		}
		printf("\n");

		d.x = d.x - xmul * is.dx;
		d.y = d.y - ymul * is.dy;
		v.x -= d.x;
		v.y -= d.y;

		printf("final dy=%g\n", d.y);

		bbox.move(d.x, d.y);
	}

	dofall(lvl, fallis);
}

Point Body::step(const Point &v) {
	Point loc(bbox.a);
	Point d(tillwhole(loc.x, v.x), tillwhole(loc.y, v.y));

	if (d.x == 0.0 && v.x != 0.0)
		d.x = fabs(v.x) / v.x;

	if (fabs(d.x) > fabs(v.x))
		d.x = v.x;

	if (d.y == 0.0 && v.y != 0.0)
		d.y = fabs(v.y) / v.y;

	if (fabs(d.y) > fabs(v.y))
		d.y = v.y;

	return d;
}

void Body::dofall(const Lvl &lvl, const Isect &is) {
	Lvl::Blkinfo bi = lvl.majorblk(z, bbox);
	double g = bi.tile.gravity();

	if(vel.y > 0 && is.dy > 0 && fall) { /* hit the ground */
		/* Constantly try to fall in order to test ground
		 * beneath us. */
		acc.y = g;
		fall = false;
	} else if (vel.y < 0 && is.dy > 0) { /* hit my head on something */
		vel.y = 0;
		acc.y = g;
		fall = true;
	}

	if (!is.is && !fall) { /* are we falling now? */
		acc.y = g;
		vel.y = 0;
		fall = true;
	}

	if (fall && vel.y < Maxdy)
		vel.y += acc.y;
}

static double tillwhole(double loc, double vel)
{
	if (vel > 0) {
		int l = loc + 0.5;
		return l - loc;
	} else {
		int l = loc;
		return l - loc;
	}
}