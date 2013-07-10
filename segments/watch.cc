// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

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

class WatchUi : public Ui {
public:
	WatchUi(unsigned int, unsigned int, bool, Segments*, std::vector<Segments::Oper>);
	virtual bool frame();
	virtual void key(int, bool) { /* ignore */ }
	virtual void motion(int, int, int, int) { /* ignore */ }
	virtual void click(int, int, int, bool) { /* ignore */ }
private:
	class AnimationWrapper {
	public:
		AnimationWrapper() {}
		AnimationWrapper(const Segments::State &s, const Segments::State &e,
				 const Segments::Oper &op, const Segments::Sweep& sweep,
				 double tweenSpeed) : start(s), end(e), currentFrame(0), gripperPositions(), segmentPositions() {
			movingSegment = op.seg;
			geom2d::Pt startPoint(start.x, start.y);
			geom2d::Pt endPoint(start.lines[op.seg].midpt());

			buildGripperMovement(startPoint,endPoint,tweenSpeed);

			segmentMovementStart = gripperPositions.size();

			buildSegmentMovement(start.lines[op.seg], end.lines[op.seg],
					     op, sweep, tweenSpeed);
		}

		bool hasMoreFrames() { return currentFrame < gripperPositions.size(); }

		void advanceFrame() { currentFrame++; }

		bool animatingSegment() { return currentFrame >= segmentMovementStart; }

		unsigned int getMovingSegment() { return movingSegment; }

		Segments::State getStart() { return start; }

		geom2d::Pt currentGripperPosition() {
			return gripperPositions[currentFrame];
		}

		geom2d::LineSg currentLineSegmentPosition() {
			return currentFrame >= segmentMovementStart ?
				segmentPositions[currentFrame - segmentMovementStart] :
				segmentPositions[0];
		}

		private:
		void buildGripperMovement(const geom2d::Pt &start, const geom2d::Pt &end, double tweenSpeed) {
			gripperPositions = buildSinglePointTranslation(start,end,tweenSpeed);
		}
		void buildSegmentMovement(const geom2d::LineSg &start, const geom2d::LineSg &end,
					  const Segments::Oper &op,  const Segments::Sweep& sweep,
					  double tweenSpeed) {
			geom2d::Pt startMid = start.midpt();
			segmentPositions.push_back(start);
			gripperPositions.push_back(startMid);
			if(op.op == Segments::Oper::Move) {
				std::vector<geom2d::Pt> pts0 = buildSinglePointTranslation(start.p0, end.p0, tweenSpeed);
				std::vector<geom2d::Pt> pts1 = buildSinglePointTranslation(start.p1, end.p1, tweenSpeed);
				std::vector<geom2d::Pt> mids = buildSinglePointTranslation(start.midpt(), end.midpt(), tweenSpeed);
				for(int i = 0; i < (int)pts0.size(); i++) {
					segmentPositions.push_back(geom2d::LineSg(pts0[i],pts1[i]));
					gripperPositions.push_back(mids[i]);
				}
			}
			else if(op.op == Segments::Oper::Rotate) {
				std::vector<geom2d::Pt> pts0 = buildSingleArcRotation(sweep.arcs[0], tweenSpeed);
				std::vector<geom2d::Pt> pts1 = buildSingleArcRotation(sweep.arcs[1], tweenSpeed);
				for(int i = 0; i < (int)pts0.size(); i++) {
				 	segmentPositions.push_back(geom2d::LineSg(pts0[i],pts1[i]));
				 	gripperPositions.push_back(startMid);
				}
			}


 		}

		std::vector<geom2d::Pt> buildSinglePointTranslation(const geom2d::Pt &start, const geom2d::Pt &end, double tweenSpeed) {
			std::vector<geom2d::Pt> translation;
			double x = start.x;
			double y = start.y;
			double dx = (double)end.x - x;
			double dy = (double)end.y - y;
			double dist = sqrt(dx*dx + dy*dy);
			int framesToAdd = dist / tweenSpeed + 1;
			dx /= (double)framesToAdd;
			dy /= (double)framesToAdd;
			for(int i = 0; i < framesToAdd; i++) {
				x += dx;
				y += dy;
				translation.push_back(geom2d::Pt(x,y));
			}

			if(translation.back() != end)
				translation.push_back(end);

			return translation;
		}

		std::vector<geom2d::Pt> buildSingleArcRotation(const geom2d::Arc &arc, double tweenSpeed) {
			std::vector<geom2d::Pt> rotation;
			double dist = arc.r * arc.dt;
			int framesToAdd = dist / tweenSpeed + 1;
			double dt = arc.dt / (double)framesToAdd;
			double t = arc.t0;

			double cx = arc.c.x;
			double cy = arc.c.y;
			double r = arc.r;
			geom2d::Pt start = arc.start();
			double x = start.x;
			double y = start.y;
			rotation.push_back(geom2d::Pt(x,y));

			for(int i = 1; i < framesToAdd; i++) {
				t += dt;
				x = r * cos(t) + cx;
				y = r * sin(t) + cy;
				rotation.push_back(geom2d::Pt(x,y));
			}

			geom2d::Pt end = arc.end();
			if(rotation.back() != end)
				rotation.push_back(end);

			return rotation;
		}

		Segments::State start, end;
		unsigned int movingSegment;
		unsigned int currentFrame;
		unsigned int segmentMovementStart;
		std::vector<geom2d::Pt> gripperPositions;
		std::vector<geom2d::LineSg> segmentPositions;
	};

	Segments* instance;
	Segments::State currentState;
	Segments::State goalState;
	Segments::Sweep sweep;
	AnimationWrapper currentAnimation;
	std::vector<Segments::Oper> ops;
	std::vector<Segments::Oper>::iterator solutionIter;

	void drawGrid();
	void draw();
	AnimationWrapper buildAnimation(const Segments::State&,
					const Segments::State&,
					const Segments::Oper&,
					const Segments::Sweep&);
};

static unsigned long frametime = 20;	// in milliseconds
static unsigned long delay = 0;	// in milliseconds
static double tweenSpeed = 0.5;	// px per frame (roughly)
static bool echo;
static bool save;
static int skip;
static std::string level;
static std::vector<unsigned int> controls;

static void parseargs(int, const char*[]);
static void helpmsg(int);

int main(int argc, const char *argv[]) {
	parseargs(argc, argv);

	Segments::Solution sol = Segments::readdf(stdin, echo ? stdout : NULL);

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
	currentState = instance->initialstate();
	goalState = instance->goalstate();
	solutionIter = ops.begin();
	sweep.nlines = sweep.narcs = 0;
	glScalef(w / instance->width, h / instance->height, 1);

	for (int i = 0; i < skip; i++) {
		sweep = solutionIter->sweep(*instance, currentState);
		Segments::Edge e(*instance, currentState, *solutionIter);
		currentState = e.state;
		solutionIter++;
	}
}

bool WatchUi::frame() {
	bool hasMoreFrames = currentAnimation.hasMoreFrames();
	if (solutionIter == ops.end() && !hasMoreFrames) {
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

	if(!hasMoreFrames) {
		Segments::Edge e(*instance, currentState, *solutionIter);
		Segments::State nextState = e.state;
		sweep = solutionIter->sweep(*instance, currentState);

		currentAnimation = AnimationWrapper(currentState, nextState,
						    *solutionIter, sweep, tweenSpeed);
		currentState = nextState;
		solutionIter++;
	}

	draw();

	currentAnimation.advanceFrame();

	return true;
}

void WatchUi::draw() {
	double lineSegmentWidth = 2;
	scene.clear();

	Segments::State drawState = currentAnimation.getStart();

	drawGrid();

	// add the goals
	int i = 0;
	for(auto line = goalState.lines.begin(); line != goalState.lines.end(); line++) {
		scene.add(new Scene::Line(*line, somecolors[i % Nsomecolors], 1, Scene::Line::DASHED));
		i++;
	}

	// add the segments
	int movingSegment = currentAnimation.getMovingSegment();
	bool animatingSegment = currentAnimation.animatingSegment();
	i = -1;
	for(auto line = drawState.lines.begin(); line != drawState.lines.end(); line++) {
		i++;
		if(animatingSegment && movingSegment == i)
			continue;
		scene.add(new Scene::Line(*line, somecolors[i % Nsomecolors], lineSegmentWidth));

	}
	if(animatingSegment) {
		scene.add(new Scene::Line(currentAnimation.currentLineSegmentPosition(),
					  somecolors[currentAnimation.getMovingSegment() % Nsomecolors],
					  lineSegmentWidth));

		for (unsigned int i = 1; sweep.narcs == 0 && i < sweep.nlines; i++)
			scene.add(new Scene::Line(sweep.lines[i], Image::red, 1));

		for (unsigned int i = 0; i < sweep.narcs; i++)
			scene.add(new Scene::Arc(sweep.arcs[i], Image::red, 1));
	}
	// add the gripper

	geom2d::Pt gripper = currentAnimation.currentGripperPosition();
	Color red(1,0,0);
	scene.add(new Scene::Pt(gripper, red, 8, -1));
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
