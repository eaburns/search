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

	void act(const Lvl&, unsigned int);

	// bottom left
	Point loc(void) { return body.bbox.min; }

	const Bbox &bbox(void) const { return body.bbox; }

	unsigned int z(void) const { return body.z; }

private:
	void chngdir(unsigned int);
	void chngjmp(unsigned int);
	void trydoor(const Lvl&, unsigned int);

	static const unsigned int Maxjframes = 8;
	static const double Dex = 5;	// Initial value from mid

	double runspeed(void) const { return 2.0 + Dex / 4.0; }

	double jmpspeed(void) const { return 7.0 + Dex / 5.0; }

	Body body;
	unsigned int jframes;
};