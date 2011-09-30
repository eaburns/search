#include "gridmap.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cstddef>
#include <cerrno>
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
	fprintf(out, "type octile\n");
	fprintf(out, "height %u\n", h);
	fprintf(out, "width %u\n", w);
	fprintf(out, "map\n");
	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++)
			fputc(map[loc(x, y)], out);
		fputc('\n', out);
	}
}

void GridMap::load(FILE *in) {
	if (fscanf(in, "type octile\nheight %u\nwidth %u\nmap\n", &w, &h) != 2)
		fatal("Failed to read the map header");

	map = new unsigned char[w * h];
	for (unsigned int y = 0; y < h; y++) {
		if (fread((void*) (map + y * w), sizeof(*map), w, in) != w)
			fatal("Failed to read map line %u", y);
		int c = fgetc(in);
		if (c != '\n' && y < h - 1)
			fatal("Expected newline, got [%c]", c);
	}

	flags = new unsigned char[w * h];
	for (unsigned int i = 0; i < w * h; i++)
		flags[i] = terrain.flags[(int) map[i]];
}
