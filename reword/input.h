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

#if !defined INPUT_H
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
    NullInput() {}
	virtual ~NullInput() {}
    virtual void init() { std::cout << "NullInput (no control) initialised" << std::endl; }
	virtual bool initDone() { return true; };
    virtual ppkey::eButtonType translate(int key) { (void)(key); return ppkey::UP; }
    virtual int un_translate(ppkey::eButtonType b) { (void)(b); return 0; }
    virtual void down(ppkey::eButtonType b) { (void)(b); }
    virtual void up(ppkey::eButtonType b) { (void)(b); }
    virtual bool isPressed(ppkey::eButtonType b) const {  (void)(b); return false; }
    virtual bool repeat(ppkey::eButtonType b) {  (void)(b); return false; }
	virtual void setRepeat(ppkey::eButtonType b, Uint32 rate, Uint32 delay) { (void)(b); (void)(rate); (void)(delay); }
	virtual void clearRepeat() {}
    virtual std::string keyDescription(ppkey::eButtonType /*b*/) { return ""; }
};

//Standard input implementation
class Input : public IInput
{
public:
    Input();
	virtual ~Input();
    virtual void init();
	virtual bool initDone() { return _init; };
    virtual ppkey::eButtonType translate(int key);	        // Translate key to button
    virtual int un_translate(ppkey::eButtonType b);  // Reverse translation (from anywhere)
    virtual void down(ppkey::eButtonType b);	// Button pressed
    virtual void up(ppkey::eButtonType b);		// Button released
    virtual bool isPressed(ppkey::eButtonType b) const;	// Return true if button is pressed
    virtual bool repeat(ppkey::eButtonType b);	// Returns true if button repeats
	virtual void setRepeat(ppkey::eButtonType b, Uint32 rate, Uint32 delay);
	virtual void clearRepeat();
    virtual std::string keyDescription(ppkey::eButtonType b);

protected:
    // Initise joystick
    void initJoy(void);
	void cleanUp();

    // SDL joystick handle
    SDL_Joystick* joy;

    // Button states
    Button buttons[ppkey::NUMBER_OF_BUTTONS];

	bool _init;
};

#endif // INPUT_H
