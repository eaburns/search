#include "lvl.hpp"
#include "player.hpp"
#include "../utils/utils.hpp"
#include <boost/cstdint.hpp>
#include <SDL/SDL.h>
#include <unistd.h>	// sleep()
#include <cstdlib>

enum {
	Width = 1024,
	Height = 768,

	// from mid
	PlayerWidth = 21,
	PlayerOffx = 7,
	PlayerHeight = 29,
	PlayerOffy = 2,
};

SDL_Surface *screen;

static void init(void);
static unsigned int keys(void);
static void draw(const Lvl&, const Player&);
static void drawlvl(unsigned int z, const Lvl&);
static void drawplayer(const Player&);
static void clear(void);
static void fillrect(SDL_Rect*, Color);

int main(int argc, char *argv[]) {
	init();

	Lvl lvl(stdin);
	Player p(2 * Tile::Width + PlayerOffx, 2 * Tile::Height + PlayerOffy,
		0, PlayerWidth, PlayerHeight);

	for (unsigned int i = 0; i < 100; i++) {
		unsigned int next = SDL_GetTicks() + 20;

		draw(lvl, p);
		p.act(lvl, keys());
		SDL_PumpEvents();

		while (SDL_GetTicks() < next)
			;
	}

	return 0;
}

static void init(void) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		fatal("Failed to init SDL: %s\n", SDL_GetError());

	unsigned int flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
	screen = SDL_SetVideoMode(Width, Height, 0, flags);	
	if (!screen)
		fatal("Failed to set video mode: %s\n", SDL_GetError());

	SDL_WM_SetCaption("play", "play");
}

static unsigned int keys(void) {
	const Uint8 *state = SDL_GetKeyState(NULL);

	unsigned int keys = 0;
	if (state[SDLK_LEFT])
		keys |= Player::Left;
	if (state[SDLK_RIGHT])
		keys |= Player::Right;
	if (state[SDLK_UP])
		keys |= Player::Jump;
	if (state[SDLK_SPACE])
		keys |= Player::Act;
	if (state[SDLK_q])
		exit(0);

	return keys;
}

static void draw(const Lvl &lvl, const Player &p) {
	clear();
	drawlvl(0, lvl);
	drawplayer(p);
	SDL_Flip(screen);
}

static void drawlvl(unsigned int z, const Lvl &lvl) {
	for (unsigned int x = 0; x < lvl.width(); x++) {
	for (unsigned int y = 0; y < lvl.height(); y++) {
		Lvl::Blkinfo bi(lvl.at(x, y, z));
		SDL_Rect r;
		r.w = Tile::Width;
		r.h = Tile::Height;
		r.x = x * Tile::Width;
		r.y = y * Tile::Height;

		if (bi.tile.flags & Tile::Collide)
			fillrect(&r, Image::black);
		else if (bi.tile.flags & Tile::Water)
			fillrect(&r, Image::blue);
	}
	}
}

static void drawplayer(const Player &p) {
	SDL_Rect r;
	r.w = p.body.bbox.b.x - p.body.bbox.a.x;
	r.h = p.body.bbox.b.y - p.body.bbox.a.y;
	r.x = p.body.bbox.a.x;
	r.y = p.body.bbox.a.y;
	fillrect(&r, Image::green);
}

static void clear(void) {
	SDL_Rect r;
	r.x = r.y = 0;
	r.w = Width;
	r.h = Height;
	fillrect(&r, Image::white);
}

static void fillrect(SDL_Rect *rect, Color color) {
	unsigned char r = color.getred255();
	unsigned char g = color.getgreen255();
	unsigned char b = color.getblue255();
	boost::uint32_t c = SDL_MapRGB(screen->format, r, g, b);
	SDL_FillRect(screen, rect, c);
}