#include "lvl.hpp"
#include "geom.hpp"
#include "player.hpp"
#include "../utils/utils.hpp"
#include <SDL/SDL.h>
#include <unistd.h>	// sleep()
#include <cstdlib>

enum {
	Width = 640,
	Height = 480,

	// from mid
	PlayerWidth = 21,
	PlayerOffx = 7,
	PlayerHeight = 29,
	PlayerOffy = 2,
};

SDL_Surface *screen;
Point tr(0, 0);

static void init(void);
static unsigned int keys(void);
static void scroll(const Point&, const Point&);
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

	for ( ; ; ) {
		unsigned int next = SDL_GetTicks() + 20;

		draw(lvl, p);

		Point l0(p.loc());
		p.act(lvl, keys());
		scroll(l0, p.loc());

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

#if SDLVER == 13
static unsigned int keys(void) {
	int nkeys;
	const Uint8 *state = SDL_GetKeyboardState(&nkeys);

	unsigned int keys = 0;
	if (state[SDL_SCANCODE_LEFT])
		keys |= Player::Left;
	if (state[SDL_SCANCODE_RIGHT])
		keys |= Player::Right;
	if (state[SDL_SCANCODE_UP])
		keys |= Player::Jump;
	if (state[SDL_SCANCODE_SPACE])
		keys |= Player::Act;
	if (state[SDL_SCANCODE_Q])
		exit(0);

	return keys;
}
#else
static unsigned int keys(void) {
	int nkeys;
	const Uint8 *state = SDL_GetKeyState(&nkeys);

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
#endif

static void scroll(const Point &l0, const Point &l1) {
	Point delta(l1.x - l0.x, l1.y - l0.y);

	if ((delta.x > 0 && l1.x + tr.x > Width * 0.75) ||
		(delta.x < 0 && l1.x + tr.x < Width * 0.25))
		tr.x -= delta.x;
	if ((delta.y > 0 && l1.y + tr.y > Height * 0.75) ||
		(delta.y < 0 && l1.y + tr.y < Height * 0.25))
		tr.y -= delta.y;
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
		r.x = x * Tile::Width + tr.x;
		r.y = y * Tile::Height + tr.y;

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
	r.x = p.body.bbox.a.x + tr.x;
	r.y = p.body.bbox.a.y + tr.y;
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
	SDL_FillRect(screen, rect, SDL_MapRGB(screen->format, r, g, b));
}