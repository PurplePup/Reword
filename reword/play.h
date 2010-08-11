//play.h

//based on code by  Dave Parker drparker@freenet.co.uk

 
#ifndef _PLAY_H
#define _PLAY_H

#include "SDL.h"

#include "screen.h"	//physical screen handling class
#include "input.h"	//button/joystich handling class
#include "gamedata.h"  //shared data between screens (play classes)
#include "sprite.h"

// interface class for all play classes, one for each "screen"
// This way each new screen has its own class with screen and 
// input handling already in place.
// e.g	playMenu - to operate the menu
//		playGame - to play the actual game level
//		playHiScore - to load, edit and save high scores 
//		etc
class IPlay
{
public:
    
	//pure virtual functions - must be implemented
	///////////////////////////////////////////////

    //init the level/screen 
    virtual void init(Input * /*input*/) = 0;
    // drawing operation
    virtual void render(Screen* /*s*/) = 0;
    // other processing
    virtual void work(Input* /*input*/, float /*speedFactor*/) = 0;
    // notification of button/input state change
    virtual void button(Input* /*input*/, Input::ButtonType /*b*/) = 0;

	//virtual functions - dont need to be implemented if not needed
	////////////////////////////////////////////////////////////////

	// screen touch
	virtual void touch(Point /*pt*/) { /*do nothing*/ }
	//handle events other than buttons and touches
	virtual void handleEvent(SDL_Event &/*sdlevent*/) { /*do nothing*/ }
	//determine if the play class should exit (used by Game class)
	virtual bool running() const { return _running; }
	//quit the screen
	virtual void quit() { _running = false; }

	virtual ~IPlay() {}

protected:
	bool _init;		//must be set true to render etc
	bool _running;	//must be set true to not drop out of Game class
};


#endif //_PLAY_H

