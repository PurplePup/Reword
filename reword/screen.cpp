////////////////////////////////////////////////////////////////////
/*

File:			screen.cpp

Class impl:		Screen

Description:	A class based on the Surface class to manage drawing to the 
				screen surface and allow locking 

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
#include "screen.h"
#include "platform.h"
#include <cassert>

int Screen::_height = 0;
int Screen::_width = 0;

// Construct 16 bit colour screen of given size
Screen::Screen(int w, int h) : _init(false)
{
	assert(!(w<320 || h<240));	//reasonable minimum for existing game graphics etc
	
	_surface = SDL_SetVideoMode(w, h, SCREEN_BPP, SCREEN_SURFACE);

	if ( _surface == NULL )
	{
		setLastError("Unable to set video resolution");
	}
	else 
	{
		_width = w;
		_height = h;
		_init = true;
	}
}

Screen::~Screen()
{
	//Surface::~Surface will destroy the allocated _surface
}

// Lock screen
void Screen::lock(void)
{
    if (SDL_MUSTLOCK(_surface)) 
		if (SDL_LockSurface(_surface) < 0) 
		    return;
}

// Unlock screen
void Screen::unlock(void)
{
    if (SDL_MUSTLOCK(_surface)) 
		SDL_UnlockSurface(_surface);
}

// Update whole screen
void Screen::update(void)
{
	SDL_Flip(_surface);

//SDL_UpdateRect is like SDL_Flip(_surface), but seems fractionally slower?
//	SDL_UpdateRect(_surface, 0, 0, _surface->w, _surface->h); 
}


