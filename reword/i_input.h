#ifndef I_INPUT_H_INCLUDED
#define I_INPUT_H_INCLUDED

#include <SDL.h>

namespace ppkey
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
    a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,    //for Pandora kbd
    NUMBER_OF_BUTTONS
};
}

class IInput
{
public:
    IInput() {}
	virtual ~IInput() {}
    virtual void init() = 0;
    virtual bool initDone() = 0;
    virtual ppkey::eButtonType translate(int key) = 0;	    // Translate key to button
    virtual int un_translate(ppkey::eButtonType b) = 0;      // Reverse translation
    virtual void down(ppkey::eButtonType b) = 0;        	    // Button pressed
    virtual void up(ppkey::eButtonType b) = 0;      		    // Button released
    virtual bool isPressed(ppkey::eButtonType b) const = 0;	// Return true if button is pressed
    virtual bool repeat(ppkey::eButtonType b) = 0;      	    // Returns true if button repeats
	virtual void setRepeat(ppkey::eButtonType b, Uint32 rate, Uint32 delay) = 0; // Set a repeat speed for a key
	virtual void clearRepeat() = 0;                         // Clear any repeats set
    virtual std::string keyDescription(ppkey::eButtonType b) = 0;
};

#endif // I_INPUT_H_INCLUDED
