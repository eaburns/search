#include "body.hpp"

struct Lvl;

struct Player {

	enum {
		Left = 1<<0,
		Right = 1 << 1,
		Jump = 1 << 2,
		Act = 1 <<3,
	};

	enum {
		// from mid
		Width = 21,
		Offx = 7,
		Height = 29,
		Offy = 2,
	};

	Player(void) { }

	Player(unsigned int x, unsigned int y, unsigned int z,
			unsigned int w, unsigned int h) :
		body(x, y, z, w, h), jframes(0) { }

	Player(const Player &o) : body(o.body), jframes(o.jframes) { }

	bool operator==(const Player &o) const {
		return jframes == o.jframes && body == o.body;
	}

	void act(const Lvl&, unsigned int);

	// bottom left
	Point loc(void) { return body.bbox.min; }

	static double runspeed(void) { return 2.0 + Dex / 4.0; }

	static double jmpspeed(void) { return 7.0 + Dex / 5.0; }

	Body body;
	unsigned char jframes;

private:
	// xvel returns the x velocity given the control bits.
	double xvel(const Lvl&, unsigned int);

	void chngjmp(unsigned int);

	void trydoor(const Lvl&, unsigned int);

	static const unsigned int Maxjframes = 8;
	static const double Dex = 5;	// Initial value from mid
};