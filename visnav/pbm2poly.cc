#include "polymap.hpp"
#include "../utils/utils.hpp"
#include <cstdio>
#include <cerrno>

static bool *blkd;
static unsigned int w, h;

static void readpbm(FILE*);
static void junkcomment(FILE*);

int main() {
	readpbm(stdin);
	PolyMap p(blkd, w, h);
	p.output(stdout);
	return 0;
}

static void readpbm(FILE *in) {
	int n = 0;
	int res = fscanf(in, "P1\n%n", &n);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the pbm file");
	if (res != 0 || n != 3)
		fatal("Failed to read the pbm header");

	junkcomment(in);

	res = fscanf(in, " %u %u\n", &w, &h);
	if (res == EOF && ferror(in))
		fatalx(errno, "Failed to read the pbm file");
	if (res != 2)
		fatal("Failed to read the pbm size");

	if (std::numeric_limits<unsigned int>::max() / w < h)
		fatal("Image dimensions are too large");

	junkcomment(in);

	blkd = new bool[w * h];
	for (unsigned int i = 0; i < w * h; i++)
		blkd[i] = false;

	unsigned int x = 0, y = 0;
	while (y < h) {
		int c = fgetc(in);
		switch (c) {
		case '\n':
			junkcomment(in);
			break;
		case '1':
			blkd[x * h + y] = true;
			x++;
			break;
		case '0':
			blkd[x * h + y] = false;
			x++;
			break;
		case ' ':
			break;
		case EOF:
			fatal("Unexpected end of file");
		default:
			fatal("Unexpected character in input: '%c'", c);
		}
		if (x >= w) {
			y++;
			x = 0;
		}
	}
}

static void junkcomment(FILE *in) {
	int c = fgetc(in);
	if (c != '#') {
		ungetc(c, in);
		return;
	}
	do {
		c = fgetc(in);
	} while (c != '\n' && c != EOF);
	junkcomment(in);
}