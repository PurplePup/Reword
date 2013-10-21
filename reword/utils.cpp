////////////////////////////////////////////////////////////////////
/*

File:			utils.cpp

Class impl:		none

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

#include <SDL.h>
#include <SDL_keyboard.h>
//#include <SDL_image.h>	//for IMG_ functions
#include <SDL_events.h>


RandInt::RandInt()
{
//SDL.h may not have been included
#ifdef _SDL_H
    #ifdef WIN32
    #pragma message("RandInt: Using SDL_GetTicks() to seed random\n")
    #endif
    setSeed(SDL_GetTicks());
#else
    #ifdef WIN32
    #pragma message("RandInt: Using ctime to seed random\n")
    #endif
#endif
    m_rnd.Randomize();	//uses ctime to seed
}	//incase setSeed not called

void RandInt::setSeed(unsigned int seed) {
	m_rnd.SetRandomSeed(seed);
	m_rnd.Randomize();
}
int RandInt::random(int limit)
{
	return (int)m_rnd.Random(limit);
}
int RandInt::operator() (int limit)	//for sorting calls
{
	return (int)m_rnd.Random(limit);
}


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

//namespace ppg	//pp game functions
//{

//push a user event into the SDL event queue
void ppg::pushSDL_Event(int code, void *data1, void *data2)
{
	//push event (e.g. "end of level" or other user defined event)
    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = code;
    event.user.data1 = data1;
    event.user.data2 = data2;

    SDL_PushEvent(&event);
}

// push a key to the event queue -
// note this requires a untranslated key, not one of the Input::XXXX keys
void ppg::pushSDL_EventKey(int key)
{
	//push keyboard event instead of user pressing the key
	//allows us to automate some things
    SDL_Event event;
    //event.type = SDL_KEYDOWN;
    event.key.type = SDL_KEYDOWN;
    event.key.state = SDL_PRESSED;
    event.key.keysym.sym = (SDL_Keycode)key;

    SDL_PushEvent(&event);
}


// Draw a filled/solid rectangle
void ppg::drawSolidRect (Surface* s, int x, int y, int w, int h, const SDL_Color& c)
{
    if (!s->surface()) return;
    SDL_Rect r = { x, y, w, h };
    const Uint32 colour = SDL_MapRGB(s->surface()->format, c.r, c.g, c.b);
    SDL_FillRect(s->surface(), &r, colour);
}

// Draw a filled/solid alpha blended (transparent) rectangle
void ppg::drawSolidRectA(Surface* s, int x, int y, int w, int h, const SDL_Color& c, int iAlpha)
{
    if (!s->surface()) return;
    SDL_Rect r = { x, y, w, h };
    const Uint32 colour = SDL_MapRGBA(s->surface()->format, c.r, c.g, c.b, iAlpha);
    SDL_FillRect(s->surface(), &r, colour);
}

// Put pixel of colour c at x,y
void ppg::putPixel(Surface* s, int x, int y, Uint32 colour)
{
    if (!s->surface()) return;
	// If colour is NONE - no pixel is ploted
    if (x >=0 && x < s->width() && y >=0 && y < s->height()) // && c != Colour::NONE)
    {
		unsigned short* dst = static_cast<unsigned short*>(s->surface()->pixels);
		dst[y * s->surface()->pitch/sizeof(unsigned short) + x] = (unsigned short)colour;	//##fudge!!
    }
}

//function to blit, but not just to _screen - both source and destination surface needed
void ppg::blit_surface(SDL_Surface* source, SDL_Rect* srcRect, SDL_Surface* dest, int destX, int destY)
{
	SDL_Rect destRect = { destX, destY, 0, 0 };
	SDL_BlitSurface( source, srcRect, dest, &destRect );
}

//leave srcRect clip param 0 (or NULL) to blit whole surface or specify srcRect clip to blit
//only part of the source to the destination
//void blit_surface(SDL_Surface* source, SDL_Rect* srcRect, int destX, int destY)
//{
//	//set up destination rectangle
//	_r.x = destX;
//	_r.y = destY;
//    _r.w = _r.h = 0;	//height/width ignored by SDL_BlitSurface
//	//do it
//	SDL_BlitSurface( source, srcRect, _surface, &_r );
//}



//}	//namespace ppg

