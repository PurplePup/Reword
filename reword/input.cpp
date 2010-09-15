////////////////////////////////////////////////////////////////////
/*

File:			input.cpp

Class impl:		Input

Description:	A input handling class using the Button class

Author:			Al McLuckie (al-at-purplepup-dot-org)
				Based on framework by Dave Parker drparker@freenet.co.uk

Date:			06 April 2007

Licence:		This program is free software; you can redistribute it and/or modify
				it under the terms of the GNU General Public License as published by
				the Free Software Foundation; either version 2 of the License, or
				(at your option) any later version.

				This software is distributed in the hope that it will be useful,
				but WITHOUT ANY WARRANTY; without even the implied warranty of
				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
				GNU General Public License for more details.

				You should have received a copy of the GNU General Public License
				along with this program; if not, write to the Free Software
				Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
////////////////////////////////////////////////////////////////////

#include "global.h"
#include "input.h"
#include <iostream>
#include <SDL.h>


using namespace std;

// Input handler

Input::Input(void) : joy(0), _init(false)
{
    initJoy();
}

Input::~Input()
{
	cleanUp();
}

void Input::cleanUp()
{
	if (joy) SDL_JoystickClose(joy);
	joy = 0;
}

// Initialise joystick
void Input::initJoy(void)
{
    cleanUp();

    if (SDL_InitSubSystem (SDL_INIT_JOYSTICK) == -1)
    {
        setLastError("Unable to initialise joystick");
		return;
    }
    
    if ( SDL_NumJoysticks() > 0)
    {
		joy = SDL_JoystickOpen(0);
    }


    if (joy)
    {
		SDL_JoystickEventState (SDL_ENABLE);	
    }

	_init = (joy!=NULL);
}

// Button pressed
void Input::down(ButtonType b)
{
    buttons[b].down();
}

// Button released
void Input::up(ButtonType b)
{
    buttons[b].up();
}

// Returns true if button is pressed
bool Input::isPressed(ButtonType b) const
{
    return buttons[b].isPressed();
} 

// Translate key to buttons
Input::ButtonType Input::translate(int key)
{
    ButtonType res = NUMBER_OF_BUTTONS;
    switch (key)
    {
#if defined(PANDORA)
	case SDLK_LEFT:		res = LEFT;break;
    case SDLK_RIGHT:	res = RIGHT;break;
    case SDLK_UP:		res = UP;break;
    case SDLK_DOWN:		res = DOWN;break;
    case SDLK_HOME:		res = A;break;
    case SDLK_END:		res = B;break;
    case SDLK_PAGEDOWN:	res = X;break;
    case SDLK_PAGEUP:	res = Y;break;
    case SDLK_SPACE:	//for kbd
	case SDLK_LALT:		res = START;break;
    case SDLK_RETURN:	//for kbd
	case SDLK_LCTRL:	res = SELECT;break;
	case SDLK_RSHIFT:	res = L;break;
	case SDLK_RCTRL:	res = R;break;
    case SDLK_EQUALS:	res = VOLUP;break;
    case SDLK_MINUS:	res = VOLDOWN;break;
    case SDLK_c:		res = CLICK;break;
#else
	case SDLK_LEFT:		res = LEFT;break;
    case SDLK_RIGHT:	res = RIGHT;break;
    case SDLK_UP:		res = UP;break;
    case SDLK_DOWN:		res = DOWN;break;
    case SDLK_a:		res = A;break;
    case SDLK_b:		res = B;break;
    case SDLK_x:		res = X;break;
    case SDLK_y:		res = Y;break;
    case SDLK_SPACE:	res = START;break;
    case SDLK_RETURN:	res = SELECT;break;
	case SDLK_l:		res = L;break;
	case SDLK_r:		res = R;break;
    case SDLK_EQUALS:	res = VOLUP;break;
    case SDLK_MINUS:	res = VOLDOWN;break;
    case SDLK_c:		res = CLICK;break;
#endif
	default: break;
    }
//	std::cout << "Translate key : " << key << " into " << res << std::endl;

    return res;
}

// Returns true if button repeats
bool Input::repeat(ButtonType b)
{
    return (buttons[b].repeat());
}

void Input::setRepeat(ButtonType b, Uint32 rate, Uint32 delay)
{
	buttons[b].setRepeat(rate, delay);
}

void Input::clearRepeat()
{
	//default delay and rate to 0 - turn off all ...
	for (int i=0; i<Input::NUMBER_OF_BUTTONS; buttons[i++].setRepeat(0, 0))
		;	
}
