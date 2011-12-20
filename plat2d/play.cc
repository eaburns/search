#include "body.hpp"
#include "plat2d.hpp"
#include "../utils/utils.hpp"
#include <SDL/SDL.h>
#include <unistd.h>	// sleep()
#include <vector>
#include <cstdlib>
#include <cerrno>

enum {
	Width = 640,
	Height = 480,
};

static unsigned int frametime = 20;
static unsigned int echo = false;
static int nextctrl = -1;
static std::vector<unsigned int> controls;

SDL_Surface *screen;
Point tr(0, 0);

static void parseargs(int, const char*[]);
static void helpmsg(int);
static Lvl *getlvl(void);
static void handlepair(const char*, const char*, void*);
static void initsdl(void);
static unsigned int keys(void);
static unsigned int sdlkeys(void);
static void scroll(const Point&, const Point&);
static void draw(const Lvl&, const Player&);
static void drawlvl(unsigned int, const Lvl&);
static void drawplayer(const Player&);
static void clear(void);
static void fillrect(SDL_Rect*, Color);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);
	Lvl *lvl = getlvl();
	initsdl();

	Player p(2 * Tile::Width + Player::Offx, 2 * Tile::Height + Player::Offy,
		0, Player::Width, Player::Height);

	for ( ; ; ) {
		unsigned int next = SDL_GetTicks() + frametime;

		draw(*lvl, p);
		Point l0(p.loc());
		p.act(*lvl, keys());
		scroll(l0, p.loc());

		SDL_PumpEvents();

		while (SDL_GetTicks() < next)
			;
	}

	delete lvl;

	return 0;
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0)
			helpmsg(0);
		else if (i < argc - 1 && strcmp(argv[i], "-t") == 0)
			frametime = strtol(argv[++i], NULL, 10);
		else if (strcmp(argv[i], "-e") == 0)
			echo = true;
		else {
			printf("Unknown option %s\n", argv[i]);
			helpmsg(1);
		}
	}
}

static void helpmsg(int res) {
	puts("Usage: play <options>");
	puts("Options:");
	puts("-h	display this help message");
	puts("-e	echo the input back to the output if the input is a datafile");
	puts("-t <num>	sets the frame time in milliseconds");
	exit(res);
}

static Lvl *getlvl(void) {
	int c = fgetc(stdin);
	if (c == EOF)
		return NULL;
	ungetc(c, stdin);

	if (c != '#')
		return new Lvl(stdin);

	char *path = NULL;
	nextctrl = 0;
	dfreadpairs(stdin, handlepair, &path, echo);
	if (!path)
		fatal("No level key found");

	FILE *f = fopen(path, "r");
	if (!f)
		fatalx(errno, "Failed to open %s for reading", path);

	Lvl *lvl = new Lvl(f);
	fclose(f);
	return lvl;
}

static void handlepair(const char *key, const char *val, void *_pathp) {
	if (strcmp(key, "level") == 0) {
		char **pathp = (char**) _pathp;
		unsigned int sz = sizeof(val[0]) * (strlen(val) + 1);
		*pathp = (char*) malloc(sz);
		memcpy(*pathp, val, sz);
	} else if (strcmp(key, "controls") == 0) {
		controls = controlvec(val);
	}
}

static void initsdl(void) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		fatal("Failed to init SDL: %s\n", SDL_GetError());

	unsigned int flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
	screen = SDL_SetVideoMode(Width, Height, 0, flags);	
	if (!screen)
		fatal("Failed to set video mode: %s\n", SDL_GetError());

	SDL_WM_SetCaption("play", "play");
}

static unsigned int keys(void) {
	if (nextctrl < 0)
		return sdlkeys();
	if (nextctrl >= (int) controls.size()) {
		SDL_Delay(1000);
		exit(0);
	}
	return controls[nextctrl++];
}

#if SDLVER == 13
static unsigned int sdlkeys(void) {
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
static unsigned int sdlkeys(void) {
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
	drawlvl(p.z(), lvl);

	Lvl::Blkinfo bi = lvl.majorblk(p.z(), p.bbox());
	SDL_Rect r;
	r.w = Tile::Width;
	r.h = Tile::Height;
	r.x = bi.x * Tile::Width + tr.x;
	r.y = bi.y * Tile::Height + tr.y;
	fillrect(&r, Image::blue);

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
		else if (bi.tile.flags & Tile::Down)
			fillrect(&r, Image::red);
	}
	}
}

static void drawplayer(const Player &p) {
	SDL_Rect r;
	Bbox bbox(p.bbox());
	r.w = bbox.max.x - bbox.min.x;
	r.h = bbox.max.y - bbox.min.y;
	r.x = bbox.min.x + tr.x;
	r.y = bbox.min.y + tr.y;
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
	SDL_FillRect(screen, rect, SDL_MapRGB(screen->format, b, g, r));
}