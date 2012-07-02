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
void Input::down(ppkey::eButtonType b)
{
    buttons[b].down();
}

// Button released
void Input::up(ppkey::eButtonType b)
{
    buttons[b].up();
}

// Returns true if button is pressed
bool Input::isPressed(ppkey::eButtonType b) const
{
    return buttons[b].isPressed();
}

// Translate key to buttons
ppkey::eButtonType Input::translate(int key)
{
    if (key >= SDLK_a && key <= SDLK_z)
    {
        //work out what kdb letter (a-z) pressed
        return static_cast<ppkey::eButtonType>(ppkey::a + (key - SDLK_a));
    }

    ppkey::eButtonType res = ppkey::NUMBER_OF_BUTTONS;

#if defined(PANDORA)
    switch (key)
    {
    case SDLK_LEFT:		res = ppkey::LEFT;      break;
    case SDLK_RIGHT:	res = ppkey::RIGHT;     break;
    case SDLK_UP:		res = ppkey::UP;        break;
    case SDLK_DOWN:		res = ppkey::DOWN;      break;
    case SDLK_HOME:		res = ppkey::A;         break;
    case SDLK_SPACE:	//for kbd
    case SDLK_END:		res = ppkey::B;         break;
    case SDLK_RETURN:	//for kbd
    case SDLK_PAGEDOWN:	res = ppkey::X;         break;
    case SDLK_PAGEUP:	res = ppkey::Y;         break;
    case SDLK_LALT:		res = ppkey::START;     break;
    case SDLK_LCTRL:	res = ppkey::SELECT;    break;
    case SDLK_RSHIFT:	res = ppkey::L;         break;
    case SDLK_RCTRL:	res = ppkey::R;         break;
    case SDLK_EQUALS:	res = ppkey::VOLUP;     break;
    case SDLK_MINUS:	res = ppkey::VOLDOWN;   break;
    default: break;
    }
#else
    switch (key)
    {
	case SDLK_LEFT:		res = ppkey::LEFT;      break;
    case SDLK_RIGHT:	res = ppkey::RIGHT;     break;
    case SDLK_UP:		res = ppkey::UP;        break;
    case SDLK_DOWN:		res = ppkey::DOWN;      break;
    case SDLK_TAB:		res = ppkey::A;         break;       //switch
    case SDLK_SPACE:	res = ppkey::B;         break;       //selection
    case SDLK_RETURN:	res = ppkey::X;         break;       //test
    case SDLK_PAGEUP:   res = ppkey::Y;         break;       //undo
    case SDLK_PAUSE:	res = ppkey::START;     break;
    case SDLK_BACKSPACE:res = ppkey::SELECT;    break;
	case SDLK_PAGEDOWN:	res = ppkey::L;         break;
//	case SDLK_PAGEDOWN:	res = ppkey::R;         break;
    case SDLK_EQUALS:	res = ppkey::VOLUP;     break;
    case SDLK_MINUS:	res = ppkey::VOLDOWN;   break;
#if defined (GP2X)
    case SDLK_c:		res = ppkey::CLICK;     break;
#endif
	default: break;
    }
#endif
//	std::cout << "Translate key : " << key << " into " << res << std::endl;

    return res;
}

// Translate button to key (reverse of normal translate fn)
int Input::un_translate(ppkey::eButtonType b)
{
    if (b >= ppkey::a && b <= ppkey::z)
    {
        //work out what kdb letter (a-z) pressed
        return SDLK_a + (b - ppkey::a);
    }

    int key = 0;
#if defined(PANDORA)
    switch (b)
    {
    case ppkey::LEFT:	        key = SDLK_LEFT;        break;
    case ppkey::RIGHT:	        key = SDLK_RIGHT;       break;
    case ppkey::UP:		        key = SDLK_UP;          break;
    case ppkey::DOWN:		    key = SDLK_DOWN;        break;
    case ppkey::A:	    	    key = SDLK_HOME;        break;
    case ppkey::B:		        key = SDLK_SPACE;       break;
    case ppkey::X:	            key = SDLK_RETURN;      break;
    case ppkey::Y:	            key = SDLK_PAGEUP;      break;
    case ppkey::START:	        key = SDLK_LALT;        break;
    case ppkey::SELECT:         key = SDLK_LCTRL;       break;
    case ppkey::L:	            key = SDLK_RSHIFT;      break;
    case ppkey::R:         	    key = SDLK_RCTRL;       break;
    case ppkey::VOLUP:	        key = SDLK_EQUALS;      break;
    case ppkey::VOLDOWN:	    key = SDLK_MINUS;       break;
    default: break;
    }
#else
    switch (b)
    {
	case ppkey::LEFT:  		    key = SDLK_LEFT;        break;
    case ppkey::RIGHT:	        key = SDLK_RIGHT;       break;
    case ppkey::UP:		        key = SDLK_UP;          break;
    case ppkey::DOWN:		    key = SDLK_DOWN;        break;
    case ppkey::A:		        key = SDLK_TAB;         break;
    case ppkey::B:		        key = SDLK_SPACE;       break;
    case ppkey::X:		        key = SDLK_RETURN;      break;
    case ppkey::Y:		        key = SDLK_PAGEUP;      break;
    case ppkey::START:	        key = SDLK_PAUSE;       break;
    case ppkey::SELECT:	        key = SDLK_BACKSPACE;   break;
	case ppkey::L:		        key = SDLK_PAGEDOWN;    break;
	case ppkey::R:		        key = SDLK_PAGEDOWN;    break;
    case ppkey::VOLUP:	        key = SDLK_EQUALS;      break;
    case ppkey::VOLDOWN:   	    key = SDLK_MINUS;       break;
#if defined(GP2X)
    case ppkey::CLICK:	    	key = SDLK_c;           break;
#endif
	default: break;
    }
#endif
//	std::cout << "UnTranslate key : " << key << " into " << res << std::endl;

    return key;
}

std::string Input::keyDescription(ppkey::eButtonType b)
{
    std::string s;
    if (b >= ppkey::a && b <= ppkey::z)
    {
        //work out what kdb letter (a-z) pressed
        s =  char(SDLK_a + (b - ppkey::a));
        return s;
    }

#if defined(PANDORA)
    switch (b)
    {
    case ppkey::LEFT:	        s = "LEFT";     break;
    case ppkey::RIGHT:	        s = "RIGHT";    break;
    case ppkey::UP:		        s = "UP";       break;
    case ppkey::DOWN:		    s = "DOWN";     break;
    case ppkey::A:	    	    s = "(A)";      break;
    case ppkey::B:		        s = "(B)";      break;
    case ppkey::X:	            s = "(X)";      break;
    case ppkey::Y:	            s = "(Y)";      break;
    case ppkey::START:	        s = "START";    break;
    case ppkey::SELECT:         s = "SELECT";   break;
    case ppkey::L:	            s = "[L]";      break;
    case ppkey::R:         	    s = "[R]";      break;
    case ppkey::VOLUP:	        s = "VOLUP";    break;
    case ppkey::VOLDOWN:	    s = "VOLDOWN";  break;
	default: s = "UNKNOWN"; break;
    }
#else
    switch (b)
    {
	case ppkey::LEFT:  		    s = "LEFT";     break;
    case ppkey::RIGHT:	        s = "RIGHT";    break;
    case ppkey::UP:		        s = "UP";       break;
    case ppkey::DOWN:		    s = "DOWN";     break;
    case ppkey::A:		        s = "TAB";      break;
    case ppkey::B:		        s = "SPACE";    break;
    case ppkey::X:		        s = "RETURN";   break;
    case ppkey::Y:		        s = "PGUP";     break;
    case ppkey::START:	        s = "PAUSE";    break;
    case ppkey::SELECT:	        s = "BACKSPACE";break;
	case ppkey::L:		        s = "PGDOWN";   break;
	case ppkey::R:		        s = "PGDOWN";   break;
    case ppkey::VOLUP:	        s = "EQUALS";   break;
    case ppkey::VOLDOWN:   	    s = "MINUS";    break;
#if defined(GP2X)
    case ppkey::CLICK:	    	s = "CLICK";    break;
#endif
	default: s = "UNKNOWN"; break;
    }
#endif

//	std::cout << "key desc: " << s << std::endl;

    return s;
}

// Returns true if button repeats
bool Input::repeat(ppkey::eButtonType b)
{
    return (buttons[b].repeat());
}

void Input::setRepeat(ppkey::eButtonType b, Uint32 rate, Uint32 delay)
{
	buttons[b].setRepeat(rate, delay);
}

void Input::clearRepeat()
{
	//default delay and rate to 0 - turn off all ...
	for (int i=0; i<ppkey::NUMBER_OF_BUTTONS; buttons[i++].setRepeat(0, 0))
		;
}

