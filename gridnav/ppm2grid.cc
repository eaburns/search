// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "../utils/utils.hpp"
#include <cstdio>
#include <string>

unsigned int w, h;
FILE *in = stdin;
FILE *out = stdout;

static bool white();
static void readsize();
static std::string nextline();

int main() {
	if (nextline() != "P3")
		fatal("Expected P3");
	readsize();
	fprintf(out, "%u %u\nBoard:\n", h, w);
	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++)
			fputc(white() ? ' ' : '#', out);
		fputc('\n', out);
	}	
}

static bool white() {
	unsigned int r, g, b;
	if (fscanf(in, " %u %u %u", &r, &g, &b) != 3)
		fatal("Expected a color");
	return r == 255 && g == 255 && b == 255;
}

static void readsize() {
	auto l = nextline();
	const char *str = l.c_str();
	if (sscanf(str, "%u %u", &w, &h) != 2)
		fatal("Expected width and height, got [%s]\n", str);
}

static std::string nextline() {
	std::string line;
	do {
		auto l = readline(in);	
		if (!l)
			fatal("Unexpected EOF");
		line = *l;
	} while (line[0] == '#');
	return line;
}