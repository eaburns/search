#include "../graphics/ui.hpp"
#include "../utils/utils.hpp"
#include "plat2d.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <SDL/SDL_opengl.h>

struct Lvl;

class WatchUi : public Ui {
public:
	WatchUi(unsigned int, unsigned int, Lvl*, std::vector<unsigned int>);
	virtual void frame();
	virtual void key(int, bool) { /* ignore */ }
	virtual void motion(int, int, int, int) { /* ignore */ }
	virtual void click(int, int, int, bool) { /* ignore */ }

private:
	void move();
	void scroll(const geom2d::Pt&, const geom2d::Pt&);
	void draw();

	Lvl *lvl;
	Player p;
	geom2d::Pt tr;
	unsigned int width, height;
	std::vector<unsigned int> controls;
	std::vector<unsigned int>::iterator iter;
};

static unsigned long framerate = 20;	// in milliseconds
static unsigned long delay = 0;	// in milliseconds
static bool echo;
static std::string level;
static std::vector<unsigned int> controls;

static void parseargs(int, const char*[]);
static void helpmsg(int);
static void dfline(std::vector<std::string>&, void*);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);
	dfread(stdin, dfline, NULL, echo ? stdout : NULL);
	if (level == "")
		fatal("No level key in the datafile");
	if (controls.size() == 0)
		fatal("No controls");

	FILE *f = fopen(level.c_str(), "r");
	if (!f)
		fatalx(errno, "Failed to open %s", level.c_str());
	Lvl lvl(f);
	fclose(f);

	WatchUi ui(640, 480, &lvl, controls);
	ui.run(framerate);	
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);
		} else if (strcmp(argv[i], "-f") == 0) {
			framerate = strtol(argv[++i], NULL, 10);
			i++;
		} else if (strcmp(argv[i], "-d") == 0) {
			delay = strtol(argv[++i], NULL, 10);
			delay *= 1000;
			i++;
		} else if (strcmp(argv[i], "-e") == 0) {
			echo = true;
		} else {
			printf("Unknown option %s", argv[i]);
			helpmsg(1);
		}
	}
}

static void helpmsg(int status) {
	puts("Usage: watch [options]");
	puts("Options:");
	puts("	-h	print this help message");
	puts("	-d	delay in seconds before playing");
	puts("	-f	frame rate in milliseconds (default 20)");
	puts("	-e	echo the input to standard output");
	exit(status);
}

static void dfline(std::vector<std::string> &toks, void*) {
	if (toks[1] == "level")
		level = toks[2];
	else if (toks[1] == "controls")
		controls = controlvec(toks[2]);
}

WatchUi::WatchUi(unsigned int w, unsigned int h, Lvl *l, std::vector<unsigned int> cs) :
		Ui(w, h), lvl(l), tr(0, 0), width(w), height(h), controls(cs) {

	// flip everything over
	glTranslated(0.0, height, 0.0);
	glScaled(1.0, -1.0, 1.0);

	unsigned int x0 = lvl->width(), y0 = lvl->height();
	for (unsigned int x = 0; x < lvl->width(); x++) {
	for (unsigned int y = 0; y < lvl->height(); y++) {
		if (lvl->at(x, y).tile.flags & Tile::Up) {
			x0 = x;
			y0 = y;
		}
	}
	}
	if (x0 == lvl->width() || y0 == lvl->height())
		fatal("No start location");
	p = Player(x0 * Tile::Width + Player::Offx, y0 * Tile::Height + Player::Offy,
		Player::Width, Player::Height);
	geom2d::Pt startloc(x0 * Tile::Width + Player::Offx, y0 * Tile::Height + Player::Offy);
	scroll(geom2d::Pt(width/2, height/2), startloc);

	iter = controls.begin();
}

void WatchUi::frame() {
	if (delay > 0) {
		delay -= framerate;
		return;
	}

	move();
	draw();
}

void WatchUi::move() {
	if (iter == controls.end())
		exit(0);

	geom2d::Pt p0(p.loc());
	p.act(*lvl, *iter++);
	scroll(p0, p.loc());
}

void WatchUi::scroll(const geom2d::Pt &p0, const geom2d::Pt &p1) {
	geom2d::Pt delta(p1.x - p0.x, p1.y - p0.y);
	if ((delta.x > 0 && p1.x + tr.x > width * 0.75) ||
		(delta.x < 0 && p1.x + tr.x < width * 0.25))
		tr.x -= delta.x;
	if ((delta.y > 0 && p1.y + tr.y > height * 0.75) ||
		(delta.y < 0 && p1.y + tr.y < height * 0.25))
		tr.y -= delta.y;
}

void WatchUi::draw() {
	scene.clear();

	for (unsigned int x = 0; x < lvl->width(); x++) {
	for (unsigned int y = 0; y < lvl->height(); y++) {
		Lvl::Blkinfo bi(lvl->at(x, y));

		geom2d::Poly rect(4,
			x*Tile::Width + tr.x, y*Tile::Height + tr.y,
			(x+1)*Tile::Width + tr.x, y*Tile::Height + tr.y,
			(x+1)*Tile::Width + tr.x, (y+1)*Tile::Height + tr.y,
			x*Tile::Width + tr.x, (y+1)*Tile::Height + tr.y);

		if (bi.tile.flags & Tile::Collide)
			scene.add(new Scene::Poly(rect, Image::black, -1));
		else if (bi.tile.flags & Tile::Water)
			scene.add(new Scene::Poly(rect, Image::blue, -1));
		else if (bi.tile.flags & Tile::Down)
			scene.add(new Scene::Poly(rect, Image::red, -1));
	}
	}

	Bbox b = p.body.bbox;
	geom2d::Poly rect(4,
		b.min.x + tr.x, b.min.y + tr.y,
		b.max.x + tr.x, b.min.y + tr.y,
		b.max.x + tr.x, b.max.y + tr.y,
		b.min.x + tr.x, b.max.y + tr.y);
	scene.add(new Scene::Poly(rect, Image::green, -1));
}
