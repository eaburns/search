#include "../graphics/ui.hpp"
#include "../graphics/image.hpp"
#include "../utils/utils.hpp"
#include "segments.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <SDL/SDL_opengl.h>

struct Lvl;

class WatchUi : public Ui {
public:
	WatchUi(unsigned int, unsigned int, bool, Segments*, std::vector<Segments::Oper>);
	virtual bool frame();
	virtual void key(int, bool) { /* ignore */ }
	virtual void motion(int, int, int, int) { /* ignore */ }
	virtual void click(int, int, int, bool) { /* ignore */ }
private:
	Segments* instance;
	Segments::State curState;
	Segments::Sweep sweep;
	std::vector<Segments::Oper> ops;
	std::vector<Segments::Oper>::iterator iter;
	void drawGrid();
	void draw();
};

static unsigned long frametime = 20;	// in milliseconds
static unsigned long delay = 0;	// in milliseconds
static bool echo;
static bool save;
static int skip;
static std::string level;
static std::vector<unsigned int> controls;

static void parseargs(int, const char*[]);
static void helpmsg(int);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);

	Solution sol = readdf(stdin, echo ? stdout : NULL);

	Segments segs(sol.width, sol.height, sol.nangles, sol.segs);

	WatchUi ui(800, 600, save, &segs, sol.ops);
	ui.run(frametime);	
}

static void parseargs(int argc, const char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			helpmsg(0);

		} else if (i < argc-1 && strcmp(argv[i], "-f") == 0) {
			frametime = strtol(argv[++i], NULL, 10);

		} else if (i < argc-1 && strcmp(argv[i], "-d") == 0) {
			delay = strtol(argv[++i], NULL, 10);
			delay *= 1000;

		} else if (strcmp(argv[i], "-e") == 0) {
			echo = true;

		} else if (strcmp(argv[i], "-s") == 0) {
			save = true;

		} else if (i < argc-1 && strcmp(argv[i], "-skip") == 0) {
			skip = strtol(argv[++i], NULL, 10);

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
	puts("	-skip	 skips initial portion of the solution");
	exit(status);
}

WatchUi::WatchUi(unsigned int w, unsigned int h, bool save,
		 Segments* segs, std::vector<Segments::Oper> o) :
  Ui(w, h, save), instance(segs), ops(o) {
	curState = instance->initialstate();
	iter = ops.begin();
	glScalef(w / instance->width, h / instance->height, 1);

	for (int i = 0; i < skip; i++) {
		sweep = iter->sweep(*instance, curState);
		Segments::Edge e(*instance, curState, *iter);
		curState = e.state;
		iter++;
	}
}

bool WatchUi::frame() {
	if (iter == ops.end()) {
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

	draw();

	sweep = iter->sweep(*instance, curState);
	Segments::Edge e(*instance, curState, *iter);
	curState = e.state;
	iter++;

	return true;
}

void WatchUi::draw() {
	scene.clear();

	drawGrid();

	// add the segments
	int i = 0;
	for(auto line = curState.lines.begin(); line != curState.lines.end(); line++) {
		scene.add(new Scene::Line(*line, somecolors[i % Nsomecolors], 4));
		i++; 
	}

	for (unsigned int i = 0; i < sweep.nlines; i++)
		scene.add(new Scene::Line(sweep.lines[i], Image::red, 1));

	for (unsigned int i = 0; i < sweep.narcs; i++)
		scene.add(new Scene::Arc(sweep.arcs[i], Image::red, 1));
	
	// add the gripper
	Color red(1,0,0);
	scene.add(new Scene::Pt(geom2d::Pt(curState.x, curState.y), red, 8, -1));
}

void WatchUi::drawGrid() {
	Color gray(0.9,0.9,0.9);
	for(int i = 0; i <= (int)instance->width; i++) {
		geom2d::LineSg seg(geom2d::Pt(i,0),geom2d::Pt(i,instance->height));
		scene.add(new Scene::Line(seg, gray, 1));
	}
	
	for(int i = 0; i <= (int)instance->height; i++) {
		geom2d::LineSg seg(geom2d::Pt(0,i),geom2d::Pt(instance->width,i));
		scene.add(new Scene::Line(seg, gray, 1));
	}
}
