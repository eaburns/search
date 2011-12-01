#include "plat2d.hpp"

const unsigned int Plat2d::Ops[] = {
	Player::Left,
	Player::Right,
	Player::Jump,
	Player::Act,
	Player::Left | Player::Jump,
	Player::Right | Player::Jump,
	Player::Left | Player::Act,
	Player::Right | Player::Act,
	Player::Jump | Player::Act,
	Player::Left | Player::Jump | Player::Act,
	Player::Right | Player::Jump | Player::Act,
};

const unsigned int Plat2d::Nops = sizeof(Plat2d::Ops) / sizeof(Plat2d::Ops[0]);

Plat2d::Plat2d(FILE *in) : lvl(in) { }

Plat2d::State Plat2d::initialstate(void) {
	return State(2 * Tile::Width + Player::Offx, 2 * Tile::Height + Player::Offy,
		0, Player::Width, Player::Height);
}