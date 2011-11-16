#include "tile.hpp"
#include "../utils/utils.hpp"

void Tile::draw(Image &img, unsigned int x, unsigned int y, Color c) {
	for (unsigned int j = y; j < y + Tile::Height; j++) {
	for (unsigned int i = x; i < x + Tile::Width; i++)
		img.set(i, j, c);
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
	tiles[' '] = Tile(0);
	tiles['#'] = Tile(Tile::Collide | Tile::Opaque);
	tiles['w'] = Tile(Tile::Water);
	tiles['>'] = Tile(Tile::Bdoor);
	tiles[')'] = Tile(Tile::Bdoor | Tile::Water);
	tiles['<'] = Tile(Tile::Fdoor);
	tiles['('] = Tile(Tile::Fdoor | Tile::Water);
	tiles['d'] = Tile(Tile::Down);
	tiles['D'] = Tile(Tile::Down | Tile::Water);
	tiles['u'] = Tile(Tile::Up);
	tiles['U'] = Tile(Tile::Up | Tile::Water);
}

const Tiles tiles;
