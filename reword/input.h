//originally based on Dave Parkers code

/***************************************************************************
 *   Copyright (C) 2006 by Dave Parker                                     *
 *                                              *
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

#include "i_input.h"

//Null input implementtaion for testing
class NullInput : public IInput
{
public:
    virtual void init() { std::cout << "NullInput (no control) initialised" << std::endl; }
	virtual bool initDone() { return true; };
    virtual pp_i::eButtonType translate(int key) { return pp_i::UP; }
    virtual int un_translate(pp_i::eButtonType b) { return 0; }
    virtual void down(pp_i::eButtonType b) {}
    virtual void up(pp_i::eButtonType b) {}
    virtual bool isPressed(pp_i::eButtonType b) const { return false; }
    virtual bool repeat(pp_i::eButtonType b) { return false; }
	virtual void setRepeat(pp_i::eButtonType b, Uint32 rate, Uint32 delay) {}
	virtual void clearRepeat() {}
};

//Standard input implementation
class Input : public IInput
{
public:
    Input();
	virtual ~Input();
    virtual void init();
	virtual bool initDone() { return _init; };
    virtual pp_i::eButtonType translate(int key);	        // Translate key to button
    virtual int un_translate(pp_i::eButtonType b);  // Reverse translation (from anywhere)
    virtual void down(pp_i::eButtonType b);	// Button pressed
    virtual void up(pp_i::eButtonType b);		// Button released
    virtual bool isPressed(pp_i::eButtonType b) const;	// Return true if button is pressed
    virtual bool repeat(pp_i::eButtonType b);	// Returns true if button repeats
	virtual void setRepeat(pp_i::eButtonType b, Uint32 rate, Uint32 delay);
	virtual void clearRepeat();

protected:
    // Initise joystick
    void initJoy(void);
	void cleanUp();

    // SDL joystick handle
    SDL_Joystick* joy;

    // Button states
    Button buttons[pp_i::NUMBER_OF_BUTTONS];

	bool _init;
};

#endif // INPUT_H
