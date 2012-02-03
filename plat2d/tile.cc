#include "tile.hpp"
#include "../utils/utils.hpp"

void Tile::draw(Image &img, unsigned int x, unsigned int y, Color c) {
	for (unsigned int j = y; j < y + Tile::Height; j++) {
	for (unsigned int i = x; i < x + Tile::Width; i++)
		img.setpixel(i, j, c);
	}
}

int Tile::read(FILE *f)
{
	int c = fgetc(f);
	if (c == EOF)
		fatal("Unexpected EOF");

	if (!tiles.istile(c))
		fatal("Invalid tile: %c", c);

	return c;
}

Tiles::Tiles(void) {
	tiles[(int) ' '] = Tile(' ', 0);
	tiles[(int) '#'] = Tile('#', Tile::Collide | Tile::Opaque);
	tiles[(int) 'w'] = Tile('w', Tile::Water);
	tiles[(int) 'd'] = Tile('d', Tile::Down);
	tiles[(int) 'D'] = Tile('D', Tile::Down | Tile::Water);
	tiles[(int) 'u'] = Tile('u', Tile::Up);
	tiles[(int) 'U'] = Tile('U', Tile::Up | Tile::Water);
}

const Tiles tiles;
