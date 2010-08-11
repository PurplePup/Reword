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

class Input : Error
{
public:

    // mapped button values
    enum ButtonType
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

    // Constructor
    Input(void);
	~Input();

	bool initDone() const { return _init; };
	void cleanUp();
    ButtonType translate(int key);		// Translate key to button
    void down(ButtonType b);	// Button pressed
    void up(ButtonType b);		// Button released
    bool isPressed(ButtonType b) const;	// Return true if button is pressed
    bool repeat(ButtonType b);	// Returns true if button repeats
	void setRepeat(ButtonType b, Uint32 rate, Uint32 delay);
	void clearRepeat();

private:
    // Initise joystick
    void initJoy(void);

    // SDL joystick handle
    SDL_Joystick* joy;

    // Button states
    Button buttons[NUMBER_OF_BUTTONS];

	bool _init;
};

#endif // INPUT_H
