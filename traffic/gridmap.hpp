	#pragma once

#include <vector>
#include <climits>
#include <cstdio>
#include <string>
#include <iostream>

void fatal(const char*, ...);

struct GridMap {

	GridMap(unsigned int w, unsigned int h) : w(w), h(h), sz(w*h) { setfourway(); }

	GridMap(FILE *f) : nmvs(0) { load(f); }

	~GridMap();

	void output(FILE* out) const {
		fprintf(out, "%d %d\n", w, h);
		for(auto obs : obstacles)
			obs.output(out);
	}

	// coord returns x,y coordinate for the given array index.
	std::pair<int,int> coord(int loc) const {
		return std::pair<int,int>(loc%w, loc / w);
	}

	// index returns the array index for the x,y coordinate.
	int index(int x, int y) const {
		return y * w + x;
	}

	// A Move contains information for a single
	// possible move to and adjacent cell of the
	// grid.
	struct Move {
		Move() : n(0) { }

		Move(const GridMap&, const char*, int, int, unsigned int, ...);

		// dx, dy give the x and y displacement of
		// this move.
		int dx, dy;

		// delta gives the loc displacement of
		// this move.
		int delta;

		// cost is √2 for diagonal moves and 1 for
		// both vertical and horizontal moves.
		double cost;

		// chk array contains the displacements
		// for cells that must be unblocked in order
		// for this move to be valid.
		struct { int dx, dy, delta; } chk[3];

		// n is the number of valid elements in
		// the chk array.
		unsigned int n;

		// Name is the human-readable name of this operator.
		const char *name;
	};

	// ok returns true if the given move is valid from the
	// given location.
	bool ok(int loc, int t,  Move &m) const {
		std::pair<unsigned int, unsigned int> cur = coord(loc);

		int x = cur.first + m.dx;
		int y = cur.second + m.dy;

		if(x < 0 || y < 0 || x >= (int)w || y >= (int)h)
		   return false;

		for(auto obs : obstacles) {
			std::pair<unsigned int, unsigned int> o = obs.positionAt(w,h,t);

			for (unsigned int i = 0; i < m.n; i++) {
				int nxt = loc + m.chk[i].delta;
				if(nxt == index(o.first, o.second)) {

					//in this domain you can get "teleported" back to the start
					//if you collide with an obstacle -- so allow a collision
					//so long as the move is valid (really only important for 
					//octile I think
					if(m.chk[i].delta != m.delta)
						return false;
				}
			}
		}

		return true;
	}

	// setoctile sets mvs to octile movement. Octile moves
	// disallow diagonal movements unless the two adjacent
	// cells are also unblocked.
	void setoctile();

	// seteightway sets mvs to eight-way movement.  Eight-way
	// moves allow diagonal even if the adjacent cells are blocked.
	void seteightway();

	// setfourway sets mvs to four-way movements.
	void setfourway();

	struct Obstacle {
		Obstacle(int x, int y, int dx, int dy) : x(x), y(y), dx(dx), dy(dy) {}

		void output(FILE* out) const {
			fprintf(out, "%d %d %d %d\n", x, y, dx, dy); 
		}

		std::pair<int, int> positionAt( int w,  int h, int t) const {
			std::pair< int, int> newPoint(x,y);
			 int x_p = abs(x + dx * t);
			 int y_p = abs(y + dy * t);

			 int x_reflections = x_p / (w-1);
			 int y_reflections = y_p/ (h-1);

			 int x_offset = x_p % (w-1);
			 int y_offset = y_p % (h-1);

			if(x_reflections % 2 == 0) newPoint.first = x_offset;
			else newPoint.first = w - x_offset - 1;

			if(y_reflections % 2 == 0) newPoint.second = y_offset;
			else newPoint.second = h - y_offset - 1;

			return newPoint;
		}
		 int x, y, dx, dy;
	};


	unsigned int w, h, sz;
	std::string file;

	std::vector<Obstacle> obstacles;

	unsigned int nmvs;
	Move mvs[8];

private:
	void load(FILE*);
};
