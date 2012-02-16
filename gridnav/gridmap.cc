#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cstddef>
#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <string>

GridMap::GridMap(std::string &fname) : map(NULL),  file(fname), nmvs(0), flags(NULL) {
	FILE *f = fopen(file.c_str(), "r");
	if (!f)
		fatalx(errno, "Unable to open %s for reading\n", file.c_str());
	load(f);
	fclose(f);
}

GridMap::~GridMap(void) {
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
	else if (std::isdigit(c))
		load_ruml(in);
	else
		readfail("Failed to read the map header, expected 't' or a digit");
}

// Leaves the start/end locations in the FILE.
void GridMap::load_ruml(FILE *in) {
	if (fscanf(in, " %u %u\nBoard:", &h, &w) != 2)
		fatal("%s: Failed to read map size", file.c_str());

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
	if (fscanf(in, " Unit\n") != 0)
		readfail("Failed to scan Unit footer");

	int c = fgetc(in);
	ungetc(c, in);
	if (c == 'E' && fscanf(in, "Eight-way") == 0)
		eightway();
	else if (c == 'F' && fscanf(in, "Four-way") == 0)
		fourway();
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

	octile();
}

void GridMap::setsize(unsigned int width, unsigned int height) {
	w = width + 2;
	h = height + 2;
	sz = w * h;
	map = new unsigned char[sz];
	flags = new unsigned char[sz];

	for (unsigned int i = 0; i < w; i++) {
		map[i] = '\0';
		map[(h-1)*w + i] = '\0';
		flags[i] = OutOfBounds;
		flags[(h-1)*w + i] = OutOfBounds;
	}
	for (unsigned int i = 0; i < h; i++) {
		map[i*w] = '\0';
		map[i*w + w - 1] = '\0';
		flags[i*w] = OutOfBounds;
		flags[i*w + w - 1] = OutOfBounds;
	}
}

GridMap::Move::Move(const GridMap &m, int _dx, int _dy, unsigned int _n, ...) :
			dx(_dx), dy(_dy), delta(dx + m.w * dy), cost(1.0), n(_n + 1) {
	if (n > sizeof(chk) / sizeof(chk[0]))
		fatal("Cannot create a move with %d checks\n", n);

	if (dx != 0 && dy != 0)
		cost = sqrt(2.0);

	va_list ap;
	va_start(ap, _n);
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

void GridMap::octile(void) {
	// This operator ordering matches Nathan's scenario costs
	// better than other orderings.  I blame floating point rounding.
	mvs[nmvs++] = Move(*this, -1,0, 0);
	mvs[nmvs++] = Move(*this, 1,0, 0);
	mvs[nmvs++] = Move(*this, 0,-1, 0);
	mvs[nmvs++] = Move(*this, -1,-1, 2, 0,-1, -1,0);
	mvs[nmvs++] = Move(*this, 1,-1, 2, 0,-1, 1,0);
	mvs[nmvs++] = Move(*this, 0,1, 0);
	mvs[nmvs++] = Move(*this, -1,1, 2, -1,0, 0,1);
	mvs[nmvs++] = Move(*this, 1,1, 2, 1,0, 0,1);
}

void GridMap::eightway(void) {
	mvs[nmvs++] = Move(*this, 1,1, 0);
	mvs[nmvs++] = Move(*this, 1,-1, 0);
	mvs[nmvs++] = Move(*this, -1,1, 0);
	mvs[nmvs++] = Move(*this, -1,-1, 0);
	fourway();
}

void GridMap::fourway(void) {
	mvs[nmvs++] = Move(*this, 0,1, 0);
	mvs[nmvs++] = Move(*this, 0,-1, 0);
	mvs[nmvs++] = Move(*this, -1,0, 0);
	mvs[nmvs++] = Move(*this, 1,0, 0);
}
