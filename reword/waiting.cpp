////////////////////////////////////////////////////////////////////
/*

File:			waiting.cpp

Class impl:		Waiting

Description:	A class wrap a timer 'tick' waiting test
				Tests if a specified amount of time has elapsed

Author:			Al McLuckie (al-at-purplepup-dot-org)

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
#include "waiting.h"


Waiting::Waiting() : _period(0), _start(0), _delay(0)
{
}

Waiting::Waiting(Uint32 period)
{
	start(period);	//start immediately
}

//start 'timer'
void Waiting::start(Uint32 period /*=0*/, Uint32 delay /*=0*/)
{
	//reuse same period if param = 0
	_period = (period)?period:_period;
	
	//Pass 0 to delay to start immediately.
	//First time through delay might be set, then always resets to 0
	//unless start() called with new delay and period params
	_delay = delay;		

	_start = SDL_GetTicks();
}

//test if done, return true if requested time has elapsed, or false if still waiting
//NOTE: returns true (ie not waiting) if rate not yet set
bool Waiting::done(bool bReset /*= false*/)
{
	bool b = ((SDL_GetTicks() - _start) > _period+_delay);
	if (bReset && b) start();	//restart wanted and is now done, so restart
	return b;
}

