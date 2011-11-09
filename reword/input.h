//based on Dave Parkers code

/***************************************************************************
 *   Copyright (C) 2006 by Dave Parker                                     *
 *   drparker@freenet.co.uk                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>
#include "button.h"
#include "error.h"
#include "platform.h"

class IInput
{
public:
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

    IInput() : _init(false) {}
	virtual ~IInput() {}

	virtual bool initDone() const { return _init; };

	virtual void cleanUp() = 0;
    virtual eButtonType translate(int key) = 0;	    // Translate key to button
    virtual int un_translate(eButtonType b) = 0;     // Reverse translation
    virtual void down(eButtonType b) = 0;        	// Button pressed
    virtual void up(eButtonType b) = 0;      		// Button released
    virtual bool isPressed(eButtonType b) const = 0;	// Return true if button is pressed
    virtual bool repeat(eButtonType b) = 0;      	// Returns true if button repeats
	virtual void setRepeat(eButtonType b, Uint32 rate, Uint32 delay) = 0;
	virtual void clearRepeat() = 0;

protected:
	bool _init;
};

class Input : public IInput
{
public:

    // Constructor
    Input();
	virtual ~Input();

	virtual void cleanUp();
    virtual eButtonType translate(int key);	        // Translate key to button
    virtual int un_translate(eButtonType b);  // Reverse translation (from anywhere)

    virtual void down(eButtonType b);	// Button pressed
    virtual void up(eButtonType b);		// Button released
    virtual bool isPressed(eButtonType b) const;	// Return true if button is pressed
    virtual bool repeat(eButtonType b);	// Returns true if button repeats
	virtual void setRepeat(eButtonType b, Uint32 rate, Uint32 delay);
	virtual void clearRepeat();

protected:
    // Initise joystick
    void initJoy(void);

    // SDL joystick handle
    SDL_Joystick* joy;

    // Button states
    Button buttons[NUMBER_OF_BUTTONS];

};

#endif // INPUT_H
