#include "plat2d.hpp"
#include "../utils/utils.hpp"

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

// encbase is the base ASCII character for our encoding.  Each byte
// of control bits is added to this base to form an ASCII byte.  Since
// the maximum possible byte value of the control bits is 15, this will
// represent everything in lower case letters.
static const char encbase = 'a';

std::string controlstr(const std::vector<unsigned int> &controls) {
	std::string bytes;
	for (unsigned int i = 0; i < controls.size(); i++)
		bytes.push_back((controls[i] & 0xFF) + encbase);
	return bytes;
}

std::vector<unsigned int> controlvec(const std::string &bytes) {
	std::vector<unsigned int> v;
	for (unsigned int i = 0; i < bytes.size(); i++)
		v.push_back(bytes[i] - encbase);
	return v;
}
