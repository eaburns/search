#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include "../utils/safeops.hpp"
#include <cstdio>
#include <cstddef>
#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <string>

GridMap::GridMap(unsigned int width, unsigned int height) :
	map(NULL), file(""), lifecost(false), nmvs(0), flags(NULL) {
	try {
		setsize(width, height);
	} catch (const safe::BadFlow<unsigned int>&) {
		fatal("Grid is too big");
	}
	setfourway();
}

GridMap::GridMap(std::string &fname) :
	map(NULL),
	file(fname),
	lifecost(false),
	nmvs(0),
	flags(NULL) {

	FILE *f = fopen(file.c_str(), "r");
	if (!f)
		fatalx(errno, "Unable to open %s for reading\n", file.c_str());
	load(f);
	fclose(f);
}

GridMap GridMap::uniform(unsigned int w, unsigned int h,
		double prob, uint64_t seed) {
	GridMap m(w, h);

	Rand rng(seed);
	for (unsigned int x = 1; x < w+1; x++) {
	for (unsigned int y = 1; y < h+1; y++) {
		int i = m.index(x, y);
		if (rng.real() < prob) {
			m.map[i] = '#';
			m.flags[i] = OutOfBounds;
		} else {
 			m.map[i] = ' ';
			m.flags[i] = Passable;
		}
	}
	}

	return m;
}

GridMap::~GridMap() {
	if (map)
		delete[] map;
	if (flags)
		delete[] flags;
}

void GridMap::readfail(const char *fmt, ...) {
	va_list ap;
 
	va_start(ap, fmt);
	unsigned int sz = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	char buf[sz+1];
	va_start(ap, fmt);
	vsnprintf(buf, sz+1, fmt, ap);
	va_end(ap);

 	if (file.size() > 0)
		fatal("%s: %s", file.c_str(), buf);
	fatal("%s", buf);
}

void GridMap::load(FILE *in) {
	int c = fgetc(in);
	ungetc(c, in);

	if (c == 't')
		load_sturtevant(in);
	else if (c == 's')
		load_seedinst(in);
	else if (std::isdigit(c))
		load_ruml(in);
	else
		readfail("Failed to read the map header, expected 't' or a digit");
}

void GridMap::load_seedinst(FILE *in) {
	unsigned long seed;
	if (sizeof(seed) < sizeof(uint64_t))
		fatal("unsigned long is too small to read 64-bit seeds");

	double prob;
	if (fscanf(in, " seed %lu %u %u %lg\n", &seed, &w, &h, &prob) != 4)
		readfail("Failed to read map header");

	int c = fgetc(in);
	ungetc(c, in);
	bool eightway = false;
	if (c == 'E' && fscanf(in, "Eightway\n") == 0)
		eightway = true;
	else if (c == 'F' && fscanf(in, "Fourway\n") == 0)
		eightway = false;
	else
		readfail("Invalid movement type");

	c = fgetc(in);
	ungetc(c, in);
	if (c == 'U' && fscanf(in, "Unit\n") == 0)
		lifecost = false;
	else if (c == 'L' && fscanf(in, "Life\n") == 0)
		lifecost = true;
	else
		readfail("Invalid cost type");

	GridMap r = uniform(w, h, prob, seed);
	w = r.w;
	h = r.h;
	sz = r.sz;
	map = r.map;
	flags = r.flags;
	r.map = NULL;
	r.flags = NULL;

	if (eightway)
		seteightway();
	else
		setfourway();
}

// Leaves the start/end locations in the FILE.
void GridMap::load_ruml(FILE *in) {
	if (fscanf(in, " %u %u\nBoard:", &h, &w) != 2)
		readfail("Failed to read map size");

	setsize(w, h);

	if (fgetc(in) != '\n')
		fatal("Expected a new line after 'Board'");

	for (unsigned int y = 1; y < h-1; y++) {
		for (unsigned int x = 1; x < w-1; x++) {
	 		unsigned int i = (h - y - 1) * w + x;
			int c = fgetc(in);
			if (c == EOF && ferror(in))
				readfail("Failed to read the board");
			if (c == EOF)
				readfail("Failed to read the board: unexpected end of file");
			map[i] = c;
			if (c == ' ')
				flags[i] = Passable;
			else if (c == '#')
				flags[i] = OutOfBounds;
			else
				readfail("x=%u, y=%u: Unexpected character [%c] in board description",
					x, y, c);
		}
		if (fgetc(in) != '\n')
			readfail("Expected a new line");
	}

	int c = fgetc(in);
	ungetc(c, in);
	if (c == 'U' && fscanf(in, " Unit\n") == 0)
		lifecost = false;
	else if (c == 'L' && fscanf(in, " Life\n") == 0)
		lifecost = true;
	else
		readfail("Invalid cost type");

	c = fgetc(in);
	ungetc(c, in);
	if (c == 'E' && fscanf(in, "Eight-way") == 0)
		seteightway();
	else if (c == 'F' && fscanf(in, "Four-way") == 0)
		setfourway();
	else
		readfail("Invalid movement type");
}

void GridMap::load_sturtevant(FILE *in) {
	if (fscanf(in, "type ") != 0)
		readfail("Failed to read the map type");

	char typ[128];
	if (!fgets(typ, sizeof(typ), in))
		readfail("Failed to read the map type");
	if (typ[strlen(typ)-1] == '\n')
		typ[strlen(typ)-1] = '\0';
	if (strcmp(typ, "octile") != 0)
		readfail("Unsupported map type [%s]", typ);

	if (fscanf(in, "height %u\nwidth %u\nmap\n", &h, &w) != 2)
		readfail("Failed to read the map header");

	setsize(w, h);

	for (unsigned int y = 1; y < h-1; y++) {
		if (fread((void*) (map + y * w + 1), sizeof(*map), w-2, in) != w-2)
			readfail("Failed to read map line %u", y);
		int c = fgetc(in);
		if (c != '\n' && y < h - 1)
			readfail("Expected newline, got [%c]",c);
	}

	const struct Terrain{
		char flags[CHAR_MAX];
		Terrain() {
			flags[(int) '.'] = Passable;
			flags[(int) 'G'] = Passable;
			flags[(int) '@'] = OutOfBounds;
			flags[(int) 'O'] = OutOfBounds;
	 		flags[(int) 'T'] = Tree;
			flags[(int) 'S'] = Swamp;
			flags[(int) 'W'] = Water;
		}
	} terrain;

	for (unsigned int y = 1; y < h-1; y++) {
		for (unsigned int x = 1; x < w-1; x++)
			flags[y*w + x] = terrain.flags[(int) map[y*w + x]];
	}

	setoctile();
}

void GridMap::setsize(unsigned int width, unsigned int height) {
	w = safe::add(width, (unsigned int) 2);
	h = safe::add(height, (unsigned int) 2);
	sz = safe::mul(w, h);
	map = new unsigned char[sz];
	flags = new unsigned char[sz];

	for (unsigned int i = 0; i < sz; i++) {
		map[i] = ' ';
		flags[i] = Passable;
	}

	for (unsigned int i = 0; i < w; i++) {
		map[index(i, 0)] = '\0';
		map[index(i, h-1)] = '\0';
		flags[index(i, 0)] = OutOfBounds;
		flags[index(i, h-1)] = OutOfBounds;
	}
	for (unsigned int i = 0; i < h; i++) {
		map[index(0, i)] = '\0';
		map[index(w-1, i)] = '\0';
		flags[index(0, i)] = OutOfBounds;
		flags[index(w-1, i)] = OutOfBounds;
	}
}

GridMap::Move::Move(const GridMap &m, int deltax, int deltay, unsigned int num, ...) :
			dx(deltax), dy(deltay), delta(dx + m.w * dy), cost(1.0), n(num + 1) {
	if (n > sizeof(chk) / sizeof(chk[0]))
		fatal("Cannot create a move with %d checks\n", n);

	if (dx != 0 && dy != 0)
		cost = sqrt(2.0);

	va_list ap;
	va_start(ap, num);
	chk[0].dx = dx;
	chk[0].dy = dy;
	chk[0].delta = delta;
	for (unsigned int i = 1; i < n; i++) {
		chk[i].dx = va_arg(ap, int);
		chk[i].dy = va_arg(ap, int);
		chk[i].delta = chk[i].dx + m.w * chk[i].dy;
	}
	va_end(ap);
}

void GridMap::setoctile() {
	// This operator ordering seems to give more accurate
	// path-costs (compared to Nathan's scenario costs)
	// when simply using doubles as the cost type.
	nmvs = 0;
	mvs[nmvs++] = Move(*this, -1,0, 0);
	mvs[nmvs++] = Move(*this, 1,0, 0);
	mvs[nmvs++] = Move(*this, 0,-1, 0);
	mvs[nmvs++] = Move(*this, -1,-1, 2, 0,-1, -1,0);
	mvs[nmvs++] = Move(*this, 1,-1, 2, 0,-1, 1,0);
	mvs[nmvs++] = Move(*this, 0,1, 0);
	mvs[nmvs++] = Move(*this, -1,1, 2, -1,0, 0,1);
	mvs[nmvs++] = Move(*this, 1,1, 2, 1,0, 0,1);
}

void GridMap::seteightway() {
	nmvs = 0;
	mvs[nmvs++] = Move(*this, 1,1, 0);
	mvs[nmvs++] = Move(*this, 1,-1, 0);
	mvs[nmvs++] = Move(*this, -1,1, 0);
	mvs[nmvs++] = Move(*this, -1,-1, 0);
	mvs[nmvs++] = Move(*this, 0,1, 0);
	mvs[nmvs++] = Move(*this, 0,-1, 0);
	mvs[nmvs++] = Move(*this, -1,0, 0);
	mvs[nmvs++] = Move(*this, 1,0, 0);
}

void GridMap::setfourway() {
	nmvs = 0;
	mvs[nmvs++] = Move(*this, 0,1, 0);
	mvs[nmvs++] = Move(*this, 0,-1, 0);
	mvs[nmvs++] = Move(*this, -1,0, 0);
	mvs[nmvs++] = Move(*this, 1,0, 0);
}
