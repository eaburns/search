#include "plat2d.hpp"
#include "../utils/utils.hpp"

const unsigned int Plat2d::Ops[] = {
	Player::Left,
	Player::Right,
	Player::Jump,
	Player::Left | Player::Jump,
	Player::Right | Player::Jump,
};

const unsigned int Plat2d::Nops = sizeof(Plat2d::Ops) / sizeof(Plat2d::Ops[0]);

const double Plat2d::Blkwidth = Tile::Width / Player::runspeed();

const double Plat2d::Blkheight =
	Tile::Height / Player::jmpspeed() > Tile::Height / Body::Maxdy ?
		Tile::Height / Player::jmpspeed() :
		Tile::Height / Body::Maxdy;

Plat2d::Plat2d(FILE *in) : lvl(in) {
	gx = lvl.width();
	gy = lvl.height();
	for (unsigned int x = 0; x < lvl.width(); x++) {
	for (unsigned int y = 0; y < lvl.height(); y++) {
		if (!(lvl.at(x, y).tile.flags & Tile::Down))
			continue;
		if (gx < lvl.width() || gy < lvl.height())
			fatal("There are multiple goal locations in the level");
		gx = x;
		gy = y;
	}
	}
	if (gx >= lvl.width() || gy >= lvl.height())
		fatal("No goal location in the level");
}

Plat2d::State Plat2d::initialstate(void) {
	return State(2 * Tile::Width + Player::Offx, 2 * Tile::Height + Player::Offy,
		0, Player::Width, Player::Height);
}

std::string controlstr(const std::vector<unsigned int> &controls) {
	std::string bytes;
	for (unsigned int i = 0; i < controls.size(); i++)
		bytes.push_back(controls[i] & 0xFF);
	return base64enc(runlenenc(bytes));
}

std::vector<unsigned int> controlvec(const std::string &enc) {
	std::string bytes = runlendec(base64dec(enc));
	std::vector<unsigned int> v;
	for (unsigned int i = 0; i < bytes.size(); i++)
		v.push_back(bytes[i]);
	return v;
}
