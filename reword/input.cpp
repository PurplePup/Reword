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

Input::Input() : joy(0), _init(false)
{
    initJoy();
}

Input::~Input()
{
	cleanUp();
}

void Input::init()
{
    std::cout << "Standard input initialised" << std::endl;
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
//        setLastError("Unable to initialise joystick");    //TODO - reimplement error or throw
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
void Input::down(pp_i::eButtonType b)
{
    buttons[b].down();
}

// Button released
void Input::up(pp_i::eButtonType b)
{
    buttons[b].up();
}

// Returns true if button is pressed
bool Input::isPressed(pp_i::eButtonType b) const
{
    return buttons[b].isPressed();
}

// Translate key to buttons
pp_i::eButtonType Input::translate(int key)
{
    pp_i::eButtonType res = pp_i::NUMBER_OF_BUTTONS;
    switch (key)
    {
#if defined(PANDORA)
	case SDLK_LEFT:		res = pp_i::LEFT;break;
    case SDLK_RIGHT:	res = pp_i::RIGHT;break;
    case SDLK_UP:		res = pp_i::UP;break;
    case SDLK_DOWN:		res = pp_i::DOWN;break;
    case SDLK_HOME:		res = pp_i::A;break;
    case SDLK_END:		res = pp_i::B;break;
    case SDLK_PAGEDOWN:	res = pp_i::X;break;
    case SDLK_PAGEUP:	res = pp_i::Y;break;
    case SDLK_SPACE:	//for kbd
	case SDLK_LALT:		res = pp_i::START;break;
    case SDLK_RETURN:	//for kbd
	case SDLK_LCTRL:	res = pp_i::SELECT;break;
	case SDLK_RSHIFT:	res = pp_i::L;break;
	case SDLK_RCTRL:	res = pp_i::R;break;
    case SDLK_EQUALS:	res = pp_i::VOLUP;break;
    case SDLK_MINUS:	res = pp_i::VOLDOWN;break;
    case SDLK_c:		res = pp_i::CLICK;break;
#else
	case SDLK_LEFT:		res = pp_i::LEFT;break;
    case SDLK_RIGHT:	res = pp_i::RIGHT;break;
    case SDLK_UP:		res = pp_i::UP;break;
    case SDLK_DOWN:		res = pp_i::DOWN;break;
    case SDLK_a:		res = pp_i::A;break;
    case SDLK_b:		res = pp_i::B;break;
    case SDLK_x:		res = pp_i::X;break;
    case SDLK_y:		res = pp_i::Y;break;
    case SDLK_SPACE:	res = pp_i::START;break;
    case SDLK_RETURN:	res = pp_i::SELECT;break;
	case SDLK_l:		res = pp_i::L;break;
	case SDLK_r:		res = pp_i::R;break;
    case SDLK_EQUALS:	res = pp_i::VOLUP;break;
    case SDLK_MINUS:	res = pp_i::VOLDOWN;break;
    case SDLK_c:		res = pp_i::CLICK;break;
#endif
	default: break;
    }
//	std::cout << "Translate key : " << key << " into " << res << std::endl;

    return res;
}

// Translate button to key (reverse of normal translate fn)
int Input::un_translate(pp_i::eButtonType b)
{
    int key = 0;
    switch (b)
    {
#if defined(PANDORA)
	case pp_i::LEFT:	        key = SDLK_LEFT;    break;
    case pp_i::RIGHT:	        key = SDLK_RIGHT;   break;
    case pp_i::UP:		        key = SDLK_UP;      break;
    case pp_i::DOWN:		    key = SDLK_DOWN;    break;
    case pp_i::A:	    	    key = SDLK_HOME;    break;
    case pp_i::B:		        key = SDLK_END;     break;
    case pp_i::X:	            key = SDLK_PAGEDOWN;break;
    case pp_i::Y:	            key = SDLK_PAGEUP;  break;
    case pp_i::START:	        key = SDLK_SPACE;   break;
    case pp_i::SELECT:          key = SDLK_RETURN;  break;
	case pp_i::L:	            key = SDLK_RSHIFT;  break;
	case pp_i::R:         	    key = SDLK_RCTRL;   break;
    case pp_i::VOLUP:	        key = SDLK_EQUALS;  break;
    case pp_i::VOLDOWN:	        key = SDLK_MINUS;   break;
    case pp_i::CLICK:	    	key = SDLK_c;       break;
#else
	case pp_i::LEFT:  		    key = SDLK_LEFT;    break;
    case pp_i::RIGHT:	        key = SDLK_RIGHT;   break;
    case pp_i::UP:		        key = SDLK_UP;      break;
    case pp_i::DOWN:		    key = SDLK_DOWN;    break;
    case pp_i::A:		        key = SDLK_a;       break;
    case pp_i::B:		        key = SDLK_b;       break;
    case pp_i::X:		        key = SDLK_x;       break;
    case pp_i::Y:		        key = SDLK_y;       break;
    case pp_i::START:	        key = SDLK_SPACE;   break;
    case pp_i::SELECT:	        key = SDLK_RETURN;  break;
	case pp_i::L:		        key = SDLK_l;       break;
	case pp_i::R:		        key = SDLK_r;       break;
    case pp_i::VOLUP:	        key = SDLK_EQUALS;  break;
    case pp_i::VOLDOWN:   	    key = SDLK_MINUS;   break;
    case pp_i::CLICK:	    	key = SDLK_c;       break;
#endif
	default: break;
    }
//	std::cout << "UnTranslate key : " << key << " into " << res << std::endl;

    return key;
}

// Returns true if button repeats
bool Input::repeat(pp_i::eButtonType b)
{
    return (buttons[b].repeat());
}

void Input::setRepeat(pp_i::eButtonType b, Uint32 rate, Uint32 delay)
{
	buttons[b].setRepeat(rate, delay);
}

void Input::clearRepeat()
{
	//default delay and rate to 0 - turn off all ...
	for (int i=0; i<pp_i::NUMBER_OF_BUTTONS; buttons[i++].setRepeat(0, 0))
		;
}

