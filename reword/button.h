//based on Dave...

//button.h

#ifndef _BUTTON_H
#define _BUTTON_H

#include "SDL.h"

class Button
{
public:
	Button();

	void up();
	void down();
	bool isPressed() const;
	bool repeat();
	void setRepeat(Uint32 rate, Uint32 delay);

private:
	bool _pressed;	//true=pressed, false=up
	Uint32 _delay;	//repeat delay before key repeats
	Uint32 _rate;	//repeat rate to reset each time
	Uint32 _repeat;	//repeat rate
    Uint32 _last_pressed;	// Time of last press/release
};

#endif	//_BUTTON_H

