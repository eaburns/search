#include "../graphics/ui.hpp"
#include "../utils/utils.hpp"
#include "unknowngridmap.hpp"
#include "unknowngridnav.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <SDL/SDL_opengl.h>

class WatchUi : public Ui {
public:
	WatchUi(unsigned int, unsigned int, bool, UnknownGridMap*, std::vector<unsigned int>);
	virtual bool frame();
	virtual void key(int, bool) { /* ignore */ }
	virtual void motion(int, int, int, int) { /* ignore */ }
	virtual void click(int, int, int, bool) { /* ignore */ }

	geom2d::Pt loc;
	geom2d::Pt goal;

private:
	void move();
	void draw();

	UnknownGridMap *gridmap;
	unsigned int width, height;
	std::vector<unsigned int> controls;
	std::vector<unsigned int>::iterator iter;
	unsigned int currentframe;
	bool reset;
};

static unsigned long frametime = 50;	// in milliseconds
static unsigned long delay = 0;	// in milliseconds
static bool echo;
static bool save;
static std::string level;
static std::vector<unsigned int> controls;

static geom2d::Pt scale(1,1);
static geom2d::Pt start;
static geom2d::Pt goal;

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

	UnknownGridMap gridmap(f);
	fclose(f);

	fprintf(stderr, "%d %d\n", gridmap.w, gridmap.h);

	unsigned int width = 800;
	unsigned int height = 600;

	scale.x = (double)width / (double)gridmap.w;
	scale.y = (double)height / (double)gridmap.h;

	WatchUi ui(width, height, save, &gridmap, controls);
	ui.loc = start;
	ui.goal = goal;
	ui.run(frametime);
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);
		} else if (strcmp(argv[i], "-f") == 0) {
			frametime = strtol(argv[++i], NULL, 10);
			i++;
		} else if (strcmp(argv[i], "-d") == 0) {
			delay = strtol(argv[++i], NULL, 10);
			delay *= 1000;
			i++;
		} else if (strcmp(argv[i], "-e") == 0) {
			echo = true;
		} else if (strcmp(argv[i], "-s") == 0) {
			save = true;
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
	puts("	-s	saves an mpeg");
	exit(status);
}

static void dfline(std::vector<std::string> &toks, void*) {
	if (toks[1] == "level")
		level = toks[2];
	else if (toks[1] == "controls")
		controls = controlvec(toks[2]);
	else if (toks[1] == "start x")
		start.x = strtol(toks[2].c_str(), NULL, 10) + 1;
	else if (toks[1] == "start y")
		start.y = strtol(toks[2].c_str(), NULL, 10) + 1;
	else if (toks[1] == "goal x")
		goal.x = strtol(toks[2].c_str(), NULL, 10) + 1;
	else if (toks[1] == "goal y")
		goal.y = strtol(toks[2].c_str(), NULL, 10)+ 1;
}

WatchUi::WatchUi(unsigned int w, unsigned int h, bool save,
		UnknownGridMap *g, std::vector<unsigned int> cs) :
		Ui(w, h, save), gridmap(g), width(w),
		height(h), controls(cs), currentframe(0), reset(false) {

	// flip everything over
	glTranslated(0.0, height, 0.0);
	glScaled(1.0, -1.0, 1.0);

	iter = controls.begin();
}

bool WatchUi::frame() {
	if (iter == controls.end()) {
		if (delay == 0)
			delay = 1000;
		else
			delay -= frametime;
		if (delay <= 0)
			return false;
		return true;
	}
	if (delay > 0) {
		delay -= frametime;
		return true;
	}
	move();
	draw();
	return true;
}

void WatchUi::move() {
	gridmap->revealCells((int)loc.x + gridmap->w * (int)loc.y);

	loc.x += gridmap->mvs[*iter].dx;
	loc.y += gridmap->mvs[*iter].dy;
	*iter++;

	currentframe++;
}

void WatchUi::draw() {
	scene.clear();

	geom2d::Poly g(4,
		       goal.x, goal.y,
		       goal.x + 1., goal.y,
		       goal.x + 1., goal.y + 1.,
		       goal.x, goal.y + 1.
		);
	g.scale(scale.x, scale.y);
	scene.add(new Scene::Poly(g, Image::green, -1));

	geom2d::Poly l(4,
		       loc.x, loc.y,
		       loc.x + 1., loc.y,
		       loc.x + 1., loc.y + 1.,
		       loc.x, loc.y + 1.
		);
	l.scale(scale.x, scale.y);
	scene.add(new Scene::Poly(l, Image::red, -1));

	unsigned int cur = loc.x + gridmap->w * loc.y;
	unsigned int gl = goal.x + gridmap->w * goal.y;

	bool unknown, blocked;
	for(unsigned int i = 0; i < gridmap->sz; i++) {
		if(i == cur || i == gl) continue;
		
		unknown = gridmap->unknown[i];
		blocked = gridmap->blkd(i);

		int x = i % gridmap->w;
		int y = i / gridmap->w;
		geom2d::Poly cell(4,
			       (double)x, (double)y,
			       x + 1., (double)y,
			       x + 1., y + 1.,
			       (double)x, y + 1.
			);
		cell.scale(scale.x, scale.y);

		if(unknown) {
			if(blocked)
				scene.add(new Scene::Poly(cell, Color(0.2, 0.2, 0.2), -1));
			else
				scene.add(new Scene::Poly(cell, Color(0.8, 0.8, 0.8), -1));
		}
		else {
			if(blocked)
				scene.add(new Scene::Poly(cell, Image::black, -1));
			else {
				scene.add(new Scene::Poly(cell, Image::white, -1));
			}
		}
	}
}
