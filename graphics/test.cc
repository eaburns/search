// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "../utils/utils.hpp"
#include "ui.hpp"
#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <SDL/SDL.h>
#elif __linux__
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#endif

// Not sure why, but SDL_main is not found.
// Just remove the define and hope everything is OK.
#undef main

int main(int argc, char *argv[]) {
	Ui(640, 480).run(200);
	return 0;
}
