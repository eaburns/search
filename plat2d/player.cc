#include "player.hpp"
#include "lvl.hpp"

void Player::act(const Lvl &lvl, unsigned int a) {
	chngjmp(a);
	body.move(lvl, xvel(lvl, a));
	if (jframes > 0)
		jframes--;
}

double Player::xvel(const Lvl &lvl, unsigned int a) {
	double dx = 0;
	if(a & Left)
		dx -= runspeed();
	if (a & Right)
		dx += runspeed();

	Lvl::Blkinfo bi = lvl.majorblk(body.bbox);
	if (dx)
		dx = (dx < 0 ? -1 : 1) * bi.tile.drag() * runspeed();

	return dx;
}

void Player::chngjmp(unsigned int a) {
	if (!(a & Jump) && body.fall) {	// stop holding jump
		if (body.dy < 0) {
			body.dy += (Maxjframes - jframes);
			if (body.dy > 0)
				body.dy = 0;
		}
		jframes = 0;

	} else if (a & Jump && !body.fall) {
		body.dy = -jmpspeed();
		body.fall = 1;
		jframes = Maxjframes;
	}
}