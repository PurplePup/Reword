//play.h

//based on code by  Dave Parker drparker@freenet.co.uk


#ifndef _PLAY_H
#define _PLAY_H

#include "SDL.h"

#include "screen.h"	//physical screen handling class
#include "input.h"	//button/joystich handling class

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
	enum eSuccess { SU_NONE=0, SU_ARCADE, SU_GOT6, SU_BONUS, SU_BADLUCK, SU_SPEED6, SU_TIMETRIAL, SU_GAMEOVER };
	enum eState { PG_PLAY=0, PG_WAIT, PG_END, PG_DICT, PG_PAUSE }; //, PG_POPUP };
	enum eControls { CTRLID_NONE = 0, CTRLID_MENU, CTRLID_NEXT, CTRLID_PREV, CTRLID_EXIT };

	//pure virtual functions - must be implemented
	///////////////////////////////////////////////

    //init the level/screen
    virtual void init(Input * /*input*/) = 0;
    // drawing operation
    virtual void render(Screen* /*s*/) = 0;
    // other processing
    virtual void work(Input* /*input*/, float /*speedFactor*/) = 0;
    // notification of button/input state change
    virtual void button(Input* /*input*/, IInput::eButtonType /*b*/) = 0;

	//virtual functions - dont need to be implemented if not needed
	////////////////////////////////////////////////////////////////

	// screen touch (press)
	virtual bool touch(const Point &/*pt*/) { return false; /*do nothing*/ }
	// screen touch (release)
	virtual bool tap(const Point &/*pt*/) { return false; /*do nothing*/ }
    // dragging (with button or finger down) for slides/drag/highlight etc
//  virtual bool drag(const Point &/*pt*/) { return false; /*do nothing*/ }

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

#ifdef _DEBUG
public:
    void toggleDbgDisplay() { _dbg_display = !_dbg_display; }
	bool _dbg_display;  //if true, display debug info
#endif

};


#endif //_PLAY_H

