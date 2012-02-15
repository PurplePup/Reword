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
#include <SDL_image.h>	//for IMG_ functions


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

namespace ppg	//pp game functions
{

//push a user event into the SDL event queue
void pushSDL_Event(int code, void *data1, void *data2)
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
void pushSDL_EventKey(int key)
{
	//push keyboard event instead of user pressing the key
	//allows us to automate some things
    SDL_Event event;
    //event.type = SDL_KEYDOWN;
    event.key.type = SDL_KEYDOWN;
    event.key.state = SDL_PRESSED;
    event.key.keysym.sym = (SDLKey)key;

    SDL_PushEvent(&event);
}

//public draw functions
/*
//draw an outline rectangle, useful for debug
void Surface::drawRect(const Rect& r, const SDL_Color& c)
{
	const Uint32 colour = SDL_MapRGB(_surface->format, c.r, c.g, c.b);
	rectangleColor(_surface, r.left(), r.top(), r.right(), r.bottom(), colour);
}

//draw an outline rectangle, useful for debug
void Surface::drawRect(const SDL_Rect& r, const SDL_Color& c)
{
	const Uint32 colour = SDL_MapRGB(_surface->format, c.r, c.g, c.b);
	rectangleColor(_surface, r.x, r.y, r.x + r.w, r.y + r.h, colour);
}

//draw an outline rectangle, useful for debug
void Surface::drawRect(int x, int y, int w, int h, const SDL_Color& c)
{
	const Uint32 colour = SDL_MapRGB(_surface->format, c.r, c.g, c.b);
	rectangleColor(_surface, x, y, x+w, y+h, colour);
}
*/
// Draw a filled/solid rectangle
void drawSolidRect (SDL_Surface* s, int x, int y, int w, int h, const SDL_Color& c)
{
    SDL_Rect r = { x, y, w, h };
    const Uint32 colour = SDL_MapRGB(s->format, c.r, c.g, c.b);
    SDL_FillRect(s, &r, colour);
}

// Draw a filled/solid alpha blended (transparent) rectangle
void drawSolidRectA(SDL_Surface* s, int x, int y, int w, int h, const SDL_Color& c, int iAlpha)
{
    SDL_Rect r = { x, y, w, h };
    const Uint32 colour = SDL_MapRGBA(s->format, c.r, c.g, c.b, iAlpha);
    SDL_FillRect(s, &r, colour);
}

void putPixel(SDL_Surface* s, int x, int y, Uint32 colour)
{
	// Put pixel of colour c at x,y
	// If colour is NONE - no pixel is ploted
    if (x >=0 && x < s->w && y >=0 && y < s->h) // && c != Colour::NONE)
    {
		unsigned short* dst = static_cast<unsigned short*>(s->pixels);
		dst[y * s->pitch/sizeof(unsigned short) + x] = (unsigned short)colour;	//##fudge!!
    }
}

//function to blit, but not just to _screen - both source and destination surface needed
void blit_surface(SDL_Surface* source, SDL_Rect* srcRect, SDL_Surface* dest, int destX, int destY)
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

}	//namespace ppg

