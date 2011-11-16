#include "tile.hpp"
#include "../utils/utils.hpp"

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

int Tile::read(FILE *f)
{
	int c = fgetc(f);
	if (c == EOF)
		fatal("Unexpected EOF");

	if (!tiles.istile(c))
		fatal("Invalid tile: %c", c);

	return c;
}