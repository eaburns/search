#include "geom.hpp"

struct Lvl;

struct Player {

	enum {
		Left = 1<<0,
		Right = 1 << 1,
		Jump = 1 << 2,
		Act = 1 <<3,
	};

	Player(unsigned int x, unsigned int y, unsigned int z,
			unsigned int w, unsigned int h) :
		body(x, y, z, w, h), jframes(0), jmp(false) { }

	Player(const Player &o) : body(o.body), jframes(o.jframes), jmp(o.jmp) { }

	void act(const Lvl&, unsigned int);

	Body body;

private:
	void chngdir(unsigned int);
	void chngjmp(unsigned int);
	void trydoor(const Lvl&, unsigned int);

	static const unsigned int Maxjframes = 8;
	static const double Dex = 5;	// Initial value from mid

	double runspeed(void) const { return 2 + Dex / 4.0; }

	double jmpspeed(void) const { return 7 + Dex / 5.0; }

	unsigned int jframes;
	bool jmp;	// true if jump was held in previous frame
};