#include "player.hpp"
#include "lvl.hpp"
#include <vector>
#include <string>
#include <cstdio>

void fatal(const char*, ...);
extern "C" unsigned long hashbytes(unsigned char[], unsigned int);

struct Plat2d {

	static const unsigned int Ops[];
	static const unsigned int Nops;

	// Blkwidth and Blkheight define the number of frames
	// required to traverse the distance of a block in each
	// direction while traveling at the maximum speed.
	static const double Blkwidth, Blkheight;

	enum { UnitCost = true };

	typedef int Cost;
	static const int InfCost = -1;

	typedef int Oper;
	static const int Nop = -1;

	Plat2d(FILE*);

	struct State {
		State(void) { }

		State(unsigned int x, unsigned int y, unsigned int z,
			unsigned int w, unsigned int h) : player(x, y, w, h) { }

		Player player;
	};

	struct PackedState {
		// hash does nothing since the hash table
		// for plat2d states doesn't store anything.
		unsigned long hash(void) {
			unsigned int ix = x * 1e5;
			unsigned int iy = y * 1e5;
			unsigned int idy = dy * 1e5;

			static const unsigned int sz = sizeof(ix) +
				sizeof(iy) + sizeof(idy) + sizeof(jframes);
			unsigned char bytes[sz];

			unsigned int i = 0;
			for (unsigned int j = 0; j < sizeof(ix); j++) {
				bytes[i++] = ix & 0xFF;
				ix >>= 8;
			}
			for (unsigned int j = 0; j < sizeof(iy); j++) {
				bytes[i++] = iy & 0xFF;
				iy >>= 8;
			}
			for (unsigned int j = 0; j < sizeof(idy); j++) {
				bytes[i++] = idy & 0xFF;
				idy >>= 8;
			}
			bytes[i++] = jframes;
			assert (i <= sz);
			return hashbytes(bytes, i);
		}

		bool eq(PackedState &o) const {
			return jframes == o.jframes &&
				doubleeq(x, o.x) &&
				doubleeq(y, o.y) &&
				doubleeq(dy, o.dy);
		}

		double x, y, dy;
		// The body's fall flag is packed as the high-order bit
		// of jframes.
		unsigned char jframes;
	};

	struct Undo {
		Undo(State&, Oper) { }
	};

	State initialstate(void);

	Cost h(State &s) {
		return heuclidean(s);
	}

	Cost heuclidean(State &s) {
		const Point &loc = s.player.body.bbox.min;
		Point topleft(gx * Blkwidth, (gy-1) * Blkwidth);
		double min = Point::distance(loc, topleft);

		Point botleft(gx * Blkwidth, gy * Blkwidth);
		double d = Point::distance(loc, botleft);
		if (d < min)
			min = d;

		Point topright((gx+1) * Blkwidth, (gy-1) * Blkwidth);
		d = Point::distance(loc, topright);
		if (d < min)
			min = d;

		Point botright((gx+1) * Blkwidth, gy * Blkwidth);
		d = Point::distance(loc, botright);
		if (d < min)
			min = d;

		return min + 0.5;		
	}

	Cost d(State &s) {
		return 0;
	}

	bool isgoal(State &s) {
		Lvl::Blkinfo bi = lvl.majorblk(s.player.body.bbox);
		return bi.x == gx && bi.y == gy;
	}

	unsigned int nops(State &s) {
		return Nops;
	}

	Oper nthop(State &s, unsigned int n) {
		return Ops[n];
	}

	Oper revop(State &s, Oper op) {
		if (op == Player::Left)
			return Player::Right;
		if (op == Player::Right)
			return Player::Left;
		return Nop;
	}

	Cost opcost(State &s, Oper op) {
		return 1;
	}

	void undo(State &s, Undo &u) { }

	State &apply(State &buf, State &s, Oper op) {
		assert (op != Nop);
		buf = s;
		buf.player.act(lvl, (unsigned int) op);
		return buf;
	}

	void pack(PackedState &dst, State &src) {
		dst.x = src.player.body.bbox.min.x;
		dst.y = src.player.body.bbox.min.y;
		dst.dy = src.player.body.dy;
		dst.jframes = src.player.jframes;
		if (src.player.body.fall)
			dst.jframes |= 1 << 7;
	}

	State &unpack(State &buf, PackedState &pkd) {
		buf.player.jframes = pkd.jframes & 0x7F;
		buf.player.body.fall = pkd.jframes & (1 << 7);
		buf.player.body.dy = pkd.dy;
		buf.player.body.bbox.min.x = pkd.x;
		buf.player.body.bbox.min.y = pkd.y;
		buf.player.body.bbox.max.x = pkd.x + Player::Width;
		buf.player.body.bbox.max.y = pkd.y + Player::Height;
		return buf;
	}

	static void dumpstate(FILE *out, State &s) {
		fprintf(out, "%g, %g\n", s.player.loc().x, s.player.loc().y);
	}

	unsigned int gx, gy;	// goal tile location
	Lvl lvl;
};

// controlstr converts a vector of controls to an ASCII string.
std::string controlstr(const std::vector<unsigned int>&);

// controlvec converts a string of controls back into a vector.
std::vector<unsigned int> controlvec(const std::string&);
