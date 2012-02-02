#include "../utils/utils.hpp"
#include "ui.hpp"
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

int main(int argc, char *argv[]) {
	Ui(640, 480).run(200);
}