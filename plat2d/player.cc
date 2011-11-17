#include "player.hpp"
#include "lvl.hpp"

void Player::act(const Lvl &lvl, unsigned int a) {
	chngjmp(a);
	chngdir(a);

	Lvl::Blkinfo bi = lvl.majorblk(body.z, body.bbox);
	double olddx = body.vel.x;
	if (olddx)
		body.vel.x = olddx < 0 ? -1 : 1 * bi.tile.drag() * runspeed();

	double oldddy = body.acc.y;
	body.acc.y = bi.tile.gravity();

	body.move(lvl);
	body.vel.x = olddx;
	body.acc.y = oldddy;

	trydoor(lvl, a);

	if (jframes > 0)
		jframes--;
}

void Player::chngdir(unsigned int a) {
	body.vel.x = 0;
	if(a & Left)
		body.vel.x -= runspeed();
	if (a & Right)
		body.vel.x += runspeed();
}

void Player::chngjmp(unsigned int a) {
	if (jmp == (a & Jump))	// nothing changed
		return;

	jmp = a & Jump;

	if (!jmp && body.fall) {	// stop holding jump
		if (body.vel.y < 0) {
			body.vel.y += (Maxjframes - jframes);
			if (body.vel.y > 0)
				body.vel.y = 0;
		}
		jframes = 0;

	} else if (jmp && !body.fall) {
		body.vel.y = -jmpspeed();
		body.fall = 1;
		jframes = Maxjframes;
	}
}

void Player::trydoor(const Lvl &lvl, unsigned int a) {
	if (!(a & Act))
		return;

	Lvl::Blkinfo bi = lvl.majorblk(body.z, body.bbox);
	unsigned int oldz = body.z;

	if (bi.tile.flags & Tile::Bdoor)
		body.z += 1;
	else if (bi.tile.flags & Tile::Fdoor)
		body.z -= 1;

	if (body.z != oldz) {	// center on door
		Point dst(bi.x * Tile::Width, bi.y * Tile::Height);
		Point src(body.bbox.a);
		double dx = dst.x - src.x;
		double dy = dst.y - src.y;
		body.bbox.move(dx, dy);
	}
}