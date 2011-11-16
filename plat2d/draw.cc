#include "lvl.hpp"
#include "../utils/image.hpp"

const char *outfile = "level.eps";

int main(void) {
	Lvl lvl(stdin);
	Image img(
		lvl.width() * Tile::Width,
		lvl.height() * lvl.depth() * Tile::Height,
		outfile);
	lvl.draw(img);
	img.save(outfile, true);
	return 0;
}