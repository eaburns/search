#include "body.hpp"
#include "plat2d.hpp"
#include "../utils/utils.hpp"
#include <SDL/SDL.h>
#include <unistd.h>	// sleep()
#include <cstdlib>
#include <cerrno>

enum {
	Width = 640,
	Height = 480,
};

static unsigned int frametime = 20;
static unsigned int delay = 0;
static unsigned int echo = false;
static int nextctrl = -1;
static std::vector<unsigned int> controls;

SDL_Surface *screen;
Lvl *lvl;
geom2d::Pt tr(0, 0);
unsigned int startx = 2;
unsigned int starty = 2;
Player p;

static void parseargs(int, const char*[]);
static void helpmsg(int);
static void readinput();
static void findstart();
static void dfline(std::vector<std::string>&, void*);
static void initsdl();
static unsigned int keys();
static unsigned int sdlkeys();
static void scroll(const geom2d::Pt&, const geom2d::Pt&);
static void draw(const Lvl&, const Player&);
static void drawlvl(const Lvl&);
static void drawplayer(const Player&);
static void clear();
static void fillrect(SDL_Rect*, Color);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);
	readinput();
	findstart();
	p = Player(
		startx * Tile::Width + Player::Offx,
		starty * Tile::Height + Player::Offy,
		Player::Width,
		Player::Height);
	initsdl();

	SDL_Delay(delay);

	for ( ; ; ) {
		unsigned int next = SDL_GetTicks() + frametime;

		draw(*lvl, p);
		geom2d::Pt l0(p.loc());
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
	for (unsigned int i = 1; i < (unsigned int) argc; i++) {
		if (strcmp(argv[i], "-h") == 0)
			helpmsg(0);
		else if (i < (unsigned int) argc - 1 && strcmp(argv[i], "-t") == 0)
			frametime = strtol(argv[++i], NULL, 10);
		else if (strcmp(argv[i], "-e") == 0)
			echo = true;
		else if (i < (unsigned int) argc - 1 && strcmp(argv[i], "-d") == 0)
			delay = 1000 * strtol(argv[++i], NULL, 10);
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
	puts("-d <num>	set an initial delay on playing the solution");
	exit(res);
}

static void readinput() {
	int c = fgetc(stdin);
	if (c == EOF)
		return;
	ungetc(c, stdin);

	if (c != '#') {
		lvl = new Lvl(stdin);
		return;
	}

	std::string path;
	nextctrl = 0;
	dfread(stdin, dfline, &path, echo ? stdout : NULL);
	if (path.size() == 0)
		fatal("No level key found");

	FILE *f = fopen(path.c_str(), "r");
	if (!f)
		fatalx(errno, "Failed to open %s for reading", path.c_str());

	lvl = new Lvl(f);
	fclose(f);
}

static void dfline(std::vector<std::string> &cols, void *_pathp) {
	if (cols[1] == "level") {
		std::string *pathp = (std::string*) _pathp;
		*pathp = cols[2];
	} else if (cols[1] == "controls") {
		controls = controlvec(cols[2]);
	}
}

static void findstart() {
	for (unsigned int x = 0; x < lvl->width(); x++) {
	for (unsigned int y = 0; y < lvl->height(); y++) {
		if (lvl->at(x, y).tile.flags & Tile::Up) {
			startx = x;
			starty = y;
		}
	}
	}
}

static void initsdl() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		fatal("Failed to init SDL: %s\n", SDL_GetError());

	unsigned int flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
	screen = SDL_SetVideoMode(Width, Height, 0, flags);	
	if (!screen)
		fatal("Failed to set video mode: %s\n", SDL_GetError());

	SDL_WM_SetCaption("play", "play");
}

static unsigned int keys() {
	if (nextctrl < 0)
		return sdlkeys();
	if (nextctrl >= (int) controls.size()) {
		SDL_Delay(1000);
		exit(0);
	}
	return controls[nextctrl++];
}

#if SDLVER == 13
static unsigned int sdlkeys() {
	int nkeys;
	const Uint8 *state = SDL_GetKeyboardState(&nkeys);

	unsigned int keys = 0;
	if (state[SDL_SCANCODE_LEFT])
		keys |= Player::Left;
	if (state[SDL_SCANCODE_RIGHT])
		keys |= Player::Right;
	if (state[SDL_SCANCODE_UP])
		keys |= Player::Jump;
	if (state[SDL_SCANCODE_Q])
		exit(0);

	return keys;
}
#else
static unsigned int sdlkeys() {
	int nkeys;
	const Uint8 *state = SDL_GetKeyState(&nkeys);

	unsigned int keys = 0;
	if (state[SDLK_LEFT])
		keys |= Player::Left;
	if (state[SDLK_RIGHT])
		keys |= Player::Right;
	if (state[SDLK_UP])
		keys |= Player::Jump;
	if (state[SDLK_q])
		exit(0);

	return keys;
}
#endif

static void scroll(const geom2d::Pt &l0, const geom2d::Pt &l1) {
	geom2d::Pt delta(l1.x - l0.x, l1.y - l0.y);

	if ((delta.x > 0 && l1.x + tr.x > Width * 0.75) ||
		(delta.x < 0 && l1.x + tr.x < Width * 0.25))
		tr.x -= delta.x;
	if ((delta.y > 0 && l1.y + tr.y > Height * 0.75) ||
		(delta.y < 0 && l1.y + tr.y < Height * 0.25))
		tr.y -= delta.y;
}

static void draw(const Lvl &lvl, const Player &p) {
	clear();
	drawlvl(lvl);
	drawplayer(p);
	SDL_Flip(screen);
}

static void drawlvl(const Lvl &lvl) {
	for (unsigned int x = 0; x < lvl.width(); x++) {
	for (unsigned int y = 0; y < lvl.height(); y++) {
		Lvl::Blkinfo bi(lvl.at(x, y));
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
	Bbox bbox(p.body.bbox);
	r.w = bbox.max.x - bbox.min.x;
	r.h = bbox.max.y - bbox.min.y;
	r.x = bbox.min.x + tr.x;
	r.y = bbox.min.y + tr.y;
	fillrect(&r, Image::green);
}

static void clear() {
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