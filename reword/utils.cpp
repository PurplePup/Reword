////////////////////////////////////////////////////////////////////
/*

File:			utils.cpp

Class impl:		Error

Description:	Some useful utility functions. 
				Mainly string functions at the moment 

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			08 May 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4		05.12.2007	Fix stricmp call

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

#include "utils.h"
#include "platform.h"
#include "global.h"


/*
float Utils::round( float x, int places )
{
    float const shift = powf( 10.0f, places );

    x *= shift;
    x = floorf( x + 0.5f );
    x /= shift;

    return x;
} 
*/
/*
inline int Utils::round(float fl)
{
	return (int)((fl*10 + 0.5) / 10.0);
}

*/
/*
int Utils::RandomInt(unsigned int limit)
{
	return (int)m_randInt.Random(limit);
}
*/  

namespace pp_g	//pp game functions
{
  
//push an event into the SDL event queue
void pushSDL_Event(int code, void *data1, void *data2)
{
	//push event (e.g. "end of level" or other user defined event)
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = code;

	userevent.data1 = data1;
    userevent.data2 = data2;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
}

}	//namespace pp_g
