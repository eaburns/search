// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#include "scene.hpp"
#include <vector>
#include <string>

struct SDL_Surface;

struct Ui {

	Ui(unsigned int width, unsigned int height, bool record = false);

	// run runs the Ui at the given frame time, and returns
	// after a call to frame returns false.
	void run(unsigned long);

	// frame is called once per-frame to update
	// the scene.  If frame returns false, then the back
	// buffer is not swapped and the ui stops running.
	virtual bool frame();

	// key is called everytime a key is pressed or released.
	// The first argument is the key code (from SDL).
	// If the key is pressed then the second argument is
	// true otherwise it is false.
	virtual void key(int, bool);

	// motion is called whenever the mouse moves.
	// The first two arguments are the absolute coordinates
	// of the new cursor position.  The second two arguments
	// are the relative coordinates of the new position with
	// respect to the previous.
	virtual void motion(int, int, int, int);

	// click is called whenever the mouse is clicked.
	// The first arguments are the location of the click.
	// The third argument is the number of the button that
	// was pressed and the final argument is true if the
	// click was a down-click and is false if the click
	// was an up-click.
	virtual void click(int, int, int, bool);

	// chkerror checks for open GL errors and bails if
	// one was encountered with the error name and
	// the argument string.
	static void chkerror(const char*);

protected:

	SDL_Surface *screen;
	Scene scene;

private:

	// handleevents handles events and returns
	// true if it is time to quit.
	bool handleevents();

	// saveframe saves a frame to a file, returning the
	// filename.
	std::string saveframe(unsigned long);

	// If record is true then a video is created.
	bool record;
};