#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cstddef>
#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <string>

GridMap::GridMap(std::string &fname) : map(NULL), flags(NULL), file(fname) {
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

void GridMap::output(FILE *out) const {
	switch (mtype) {
	case Octile: fprintf(out, "type octile\n"); break;
	case FourWay: fprintf(out, "type four-way\n"); break;
	case EightWay: fprintf(out, "type eight-way\n"); break;
	}
	fprintf(out, "height %u\n", h);
	fprintf(out, "width %u\n", w);
	fprintf(out, "map\n");
	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++)
			fputc(map[loc(x, y)], out);
		fputc('\n', out);
	}
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

	sz = w * h;
	map = new unsigned char[sz];
	flags = new unsigned char[sz];

	if (fgetc(in) != '\n')
		fatal("Expected a new line after 'Board'");

	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
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
		mtype = EightWay;
	else if (c == 'F' && fscanf(in, "Four-way") == 0)
		mtype = FourWay;
	else
		readfail("Invalid movement type");
}

enum { Bufsz = 128 };

void GridMap::load_sturtevant(FILE *in) {
	const struct Terrain{
		char flags[CHAR_MAX];
		Terrain() {
			flags['.'] = Passable;
			flags['G'] = Passable;
			flags['@'] = OutOfBounds;
			flags['O'] = OutOfBounds;
	 		flags['T'] = Tree;
			flags['S'] = Swamp;
			flags['W'] = Water;
		}
	} terrain;

	if (fscanf(in, "type ") != 0)
		readfail("Failed to read the map type");

	char typ[Bufsz];
	if (!fgets(typ, Bufsz, in))
		readfail("Failed to read the map type");
	if (typ[strlen(typ)-1] == '\n')
		typ[strlen(typ)-1] = '\0';
	if (strcmp(typ, "octile") != 0)
		readfail("Unsupported map type [%s]", typ);

	mtype = Octile;

	if (fscanf(in, "height %u\nwidth %u\nmap\n", &h, &w) != 2)
		readfail("Failed to read the map header");

	sz = w * h;

	map = new unsigned char[sz];
	for (unsigned int y = 0; y < h; y++) {
		if (fread((void*) (map + y * w), sizeof(*map), w, in) != w)
			readfail("Failed to read map line %u", y);
		int c = fgetc(in);
		if (c != '\n' && y < h - 1)
			readfail("Expected newline, got [%c]",c);
	}

	flags = new unsigned char[sz];
	for (unsigned int i = 0; i < w * h; i++)
		flags[i] = terrain.flags[(int) map[i]];
}