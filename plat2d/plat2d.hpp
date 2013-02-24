#include "player.hpp"
#include "lvl.hpp"
#include "../visnav/visgraph.hpp"
#include <vector>
#include <string>
#include <cstdio>

void fatal(const char*, ...);
extern "C" unsigned long hashbytes(unsigned char[], unsigned int);

// controlstr converts a vector of controls to an ASCII string.
std::string controlstr(const std::vector<unsigned int>&);

// controlvec converts a string of controls back into a vector.
std::vector<unsigned int> controlvec(const std::string&);

// Maxx is the maximum travel distance in the x-direction
// in a single frame.
static const double Maxx = Runspeed;

// Maxy is the maximum travel distance in the y-direction
// in a single frame.
static const double Maxy = Jmpspeed > Body::Maxdy ? Jmpspeed : Body::Maxdy;

// W is the minimum width of a tile in 'frames'.  I.e., at max
// speed how many frames does it require to traverse the
// width of a tile.
static const double W = Tile::Width / Maxx;

// H is the minimum height of a tile in 'frames'.
static const double H = Tile::Height /  Maxy;

struct Plat2d {

	typedef int Cost;
	typedef int Oper;
	enum { Nop = -1 };

	Plat2d(FILE*);

	~Plat2d();

	struct State {
		State() : h(-1) { }

		State(unsigned int x, unsigned int y, unsigned int z,
			unsigned int w, unsigned int h) : player(x, y, w, h), h(-1) { }

		static const int K = 3;

		void vector(const Plat2d *d, double vec[]) const {
			vec[0] = player.body.bbox.min.x/d->maxx;
			vec[1] = player.body.bbox.min.y/d->maxy;
			vec[2] = (player.body.dy+Body::Maxdy)/(2*Body::Maxdy);
		}

		Player player;
		Cost h;
	};

	struct PackedState {

		bool eq(const Plat2d*, const PackedState &o) const {
			return jframes == o.jframes &&
				geom2d::doubleeq(x, o.x) &&
				geom2d::doubleeq(y, o.y) &&
				geom2d::doubleeq(dy, o.dy);
		}	
	
		unsigned long hash(const Plat2d*) {
			static const unsigned int sz = sizeof(x) +
				sizeof(y) + sizeof(dy) + sizeof(jframes);
			unsigned char bytes[sz];
			unsigned int i = 0;
			char *p = (char*) &x;
			for (unsigned int j = 0; j < sizeof(x); j++)
				bytes[i++] = p[j];
			p = (char*) &y;
			for (unsigned int j = 0; j < sizeof(y); j++)
				bytes[i++] = p[j];
			p = (char*) &dy;
			for (unsigned int j = 0; j < sizeof(dy); j++)
				bytes[i++] = p[j];
			bytes[i++] = jframes;
			assert (i <= sz);
			return hashbytes(bytes, i);
		}

		double x, y, dy;
		// The body's fall flag is packed as the high-order bit
		// of jframes.
		unsigned char jframes;
	};

	State initialstate();

	Cost h(State &s) {
		if (s.h < 0)
			s.h = hvis(s);
		return s.h;
	}

	Cost d(State &s) {
		return h(s);
	}

	bool isgoal(State &s) {
		Lvl::Blkinfo bi = lvl.majorblk(s.player.body.bbox);
		return bi.x == gx && bi.y == gy;
	}

	struct Operators {
		Operators(const Plat2d&, const State &s) : n(Nops) {
	 		// If jumping will have no effect then allow left, right and jump.
			// This is a bit of a hack, but the 'jump' action that is allowed
			// here will end up being a 'do nothing' and just fall action.
			// Effectively, we prune off the jump/left and jump/right actions
			// since they are the same as just doing left and right in this case.
			if (!s.player.canjump())
				n = 3;
		}

		unsigned int size() const {
			return n;
		}

		Oper operator[](unsigned int i) const {
			return Ops[i];
		}

	private:
		unsigned int n;

		static const unsigned int Ops[];
		static const unsigned int Nops;
	};

	struct Edge {
		Cost cost;
		Oper revop;
		Cost revcost;
		State state;

		Edge(Plat2d &d, State &s, Oper op) : revop(Nop), revcost(-1), state(s) {
			state.player.act(d.lvl, (unsigned int) op);
			cost = 1; // geom2d::Pt::distance(s.player.body.bbox.min, state.player.body.bbox.min));
			if (s.player.body.bbox.min.y == state.player.body.bbox.min.y) {
				if (op == Player::Left) {
					revop = Player::Right; 
					revcost = cost;
				}
				else if (op == Player::Right) {
					revop = Player::Left;
					revcost = cost;
				}
			}
		}

	};

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

	Cost pathcost(const std::vector<State>&, const std::vector<Oper>&);

	// Drawmap returns an image of the map.
	Image *drawmap() const;

	unsigned int gx, gy;	// goal tile location
	unsigned int x0, y0;	// initial location
	Lvl lvl;

private:

	struct Node {
		int v;		// vertex ID
		int prev;	// previous along path
		long i;	// prio queue index
		double d;	// distance to goal

		static void setind(Node *n, unsigned long ind) { n->i = ind; }
		static bool pred(const Node *a, const Node *b) { return a->d < b->d; }
	};

	void initvg();

	Cost hvis(const State &s) const {
		const Lvl::Blkinfo &bi = lvl.majorblk(s.player.body.bbox);
		if (bi.x == gx && bi.y == gy)
			return 0;;

		geom2d::Pt loc(s.player.body.bbox.center());
		loc.x /= Maxx;
		loc.y /= Maxy;

		int c = centers[bi.x * lvl.height() + bi.y];
		geom2d::Pt g = goalpt(bi, loc);
		if (togoal[c].prev == gcenter || vg->map.isvisible(loc, g)) {
			double d = geom2d::Pt::distance(loc, g);
			return d <= 0 ? 1 : ceil(d);
		}

		// Length of a tile diagonal, subtracted from the visnav
		// distance to account for the fact that the goal vertex
		// is in the center of the goal cell, not on the side.
		static const double diag = sqrt((W/2)*(W/2) + (H/2)*(H/2));
		double h = togoal[c].d - geom2d::Pt::distance(loc, vg->verts[c].pt) - diag;
		assert (h >= -geom2d::Threshold);
		return h <= 0 ? 1 : ceil(h);	// still admissible if we go up to the next int
	}

	// goalpt returns a point in the goal cell that is closest
	// to the given location.
	geom2d::Pt goalpt(const Lvl::Blkinfo &bi, const geom2d::Pt &loc) const {
		geom2d::Pt pt;
		if (bi.y == gy)
			pt.y = loc.y;
		else if (bi.y < gy)
			pt.y = gtop;
		else
			pt.y = gbottom;
		if (bi.x == gx)
			pt.x = loc.x;
		else if (bi.x < gx)
			pt.x = gleft;
		else
			pt.x = gright;
		return pt;
	}

	// savemap draws the visibility map used for the heuristic
	// to the given file.;
	void savemap(const char*) const;

	VisGraph *vg;
	std::vector<long> centers;
	std::vector<Node> togoal;
	int gcenter;	// vertex ID of the goal center
	double gleft, gright, gtop, gbottom;

	double maxx, maxy;	// width and height in pixels.
}; 

