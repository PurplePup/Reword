////////////////////////////////////////////////////////////////////
/*

File:			button.cpp

Class impl:		Button

Description:	A simple class to wrap a single button and added repeating etc

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
#include "button.h"


Button::Button() : 
	_pressed(false), _delay(500), _rate(500), _repeat(500), _last_pressed(0)
{
}

//set button state up, and prepare next button delay before repeat
void Button::up()
{
	_repeat = _delay;
	_pressed = false;
}

//set button state down, and prepare repeat rate
void Button::down()
{
	_last_pressed = SDL_GetTicks();
	_pressed = true;
}

//return true if button is pressed
bool Button::isPressed() const
{
	return _pressed;
}

//return true if a rate is set, and button pressed and rate interval has expired since last test
bool Button::repeat()
{
	if (_rate && _pressed && (SDL_GetTicks() - _last_pressed) > _repeat)
	{
		_last_pressed = SDL_GetTicks();
		_repeat = _rate;
		return true;
	}
	return false;
}

//set milliseconds of delay before button starts repeating, and between repeats
//Set a rate of 0 to switch off repeats for this button
void Button::setRepeat(Uint32 rate, Uint32 delay)
{
	_delay = delay;
	_rate = rate;
}
