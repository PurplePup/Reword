#ifndef I_INPUT_H_INCLUDED
#define I_INPUT_H_INCLUDED

#include <SDL.h>

namespace pp_i
{
// mapped button values
enum eButtonType
{
    UP,
    UPLEFT,
    LEFT,
    DOWNLEFT,
    DOWN,
    DOWNRIGHT,
    RIGHT,
    UPRIGHT,
    START,
    SELECT,
    L,
    R,
    A,
    B,
    Y,
    X,
    VOLUP,
    VOLDOWN,
    CLICK,
    PAUSE,
    NUMBER_OF_BUTTONS
};
}

class IInput
{
public:
    virtual void init() = 0;
    virtual bool initDone() = 0;
    virtual pp_i::eButtonType translate(int key) = 0;	    // Translate key to button
    virtual int un_translate(pp_i::eButtonType b) = 0;      // Reverse translation
    virtual void down(pp_i::eButtonType b) = 0;        	    // Button pressed
    virtual void up(pp_i::eButtonType b) = 0;      		    // Button released
    virtual bool isPressed(pp_i::eButtonType b) const = 0;	// Return true if button is pressed
    virtual bool repeat(pp_i::eButtonType b) = 0;      	    // Returns true if button repeats
	virtual void setRepeat(pp_i::eButtonType b, Uint32 rate, Uint32 delay) = 0; // Set a repeat speed for a key
	virtual void clearRepeat() = 0;                         // Clear any repeats set
};

#endif // I_INPUT_H_INCLUDED