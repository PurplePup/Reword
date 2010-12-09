////////////////////////////////////////////////////////////////////
/*

File:			surface.cpp

Class impl:		Surface

Description:	A class to wrap a SDL surface. Screen and Image classes are
				derived from this.

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
#include "surface.h"
#include "utils.h"

#include <SDL_gfxPrimitives.h>

Surface::Surface() : _surface(0)
{
}

Surface::Surface(SDL_Surface *s)
{
	_surface = s;
	s->refcount++;
}

Surface::~Surface()
{
	cleanUp();
}

//create a specific size surface, used mainly by Image class
bool Surface::create(unsigned int w, unsigned int h, int iAlpha /*=-1*/)
{
	cleanUp();	//destroy any existing surface
	SDL_Surface *s;
	s = SDL_CreateRGBSurface(SCREEN_SURFACE|(iAlpha>=0)?SDL_SRCALPHA:SDL_SRCCOLORKEY, w, h, SCREEN_BPP,
//		0, 0, 0, 0);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif
	bool b = initSurface(s, iAlpha);
	return (b && _surface != NULL);
}

bool Surface::initSurface(SDL_Surface *newSurface, int iAlpha)
{
	if( newSurface != NULL )
	{
		//Create an optimized image
		if (newSurface->format->Amask && iAlpha >= 0)
		{
			 //per pixel - iAlpha: 0=transparent, 255 = full opacity
			SDL_SetAlpha(newSurface, SDL_SRCALPHA, iAlpha);
			_surface = SDL_DisplayFormatAlpha( newSurface );
		}
		else
		{
			//set the transparent pixel - doesnt affect per pixel alpha surfaces
			Uint32 key = SDL_MapRGB(newSurface->format, 255,0,255); //putrid purple
			SDL_SetColorKey(newSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY, key);
			if (iAlpha >= 0)
				SDL_SetAlpha(newSurface, SDL_SRCALPHA, iAlpha); //per surface transparency only
			else
				SDL_SetAlpha(newSurface, 0, 0); //no transparency
			_surface = SDL_DisplayFormat( newSurface );
		}

		//Free the old image
		SDL_FreeSurface( newSurface );

		return true;
	}
	return false;
}

/*
//make a deep copy (copy surface pointer etc) as assignment
//override does clone (shallow copy)
void Surface::copy(Surface &s)
{
	cleanUp();
	_surface =


}
*/
void Surface::cleanUp()
{
	if (_surface) SDL_FreeSurface(_surface);
	_surface = 0;
	_r.x = _r.y = _r.w = _r.h = 0;
}

void Surface::setSurface(SDL_Surface *s)
{
	if (s == NULL) return;
	cleanUp();
	_surface = s;
}

// Get format
SDL_PixelFormat* Surface::format(void) const
{
	if (!_surface) return 0;
    return _surface->format;
}

//get actual surface
SDL_Surface * Surface::surface(void) const
{
	if (!_surface) return 0;
	return _surface;
}

// Get width
int Surface::width(void) const
{
	if (!_surface) return 0;
    return _surface->w;
}

// Get height
int Surface::height(void) const
{
	if (!_surface) return 0;
    return _surface->h;
}

//public draw functions

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

// Draw a filled/solid rectangle
void Surface::drawSolidRect (int x, int y, int w, int h, const SDL_Color& c)
{
    _r.x = x;
    _r.y = y;
    _r.w = w;
    _r.h = h;
    const Uint32 colour = SDL_MapRGB(_surface->format, c.r, c.g, c.b);
    SDL_FillRect(_surface, &_r, colour);
}

// Draw a filled/solid alpha blended (transparent) rectangle
void Surface::drawSolidRectA(int x, int y, int w, int h, const SDL_Color& c, int iAlpha)
{
    _r.x = x;
    _r.y = y;
    _r.w = w;
    _r.h = h;
    const Uint32 colour = SDL_MapRGBA(_surface->format, c.r, c.g, c.b, iAlpha);
    SDL_FillRect(_surface, &_r, colour);
}

//void Surface::putPixel (int x, int y, const Colour& c)
void Surface::PutPixel(int x, int y, Uint32 colour)
{
	// Put pixel of colour c at x,y
	// If colour is NONE - no pixel is ploted
    if (x >=0 && x < _surface->w && y >=0 && y < _surface->h) // && c != Colour::NONE)
    {
		unsigned short* dst = static_cast<unsigned short*>(_surface->pixels);
		dst[y * _surface->pitch/sizeof(unsigned short) + x] = (unsigned short)colour;	//##fudge!!
    }
}

//[static] function to blit, but not just to _screen - both source and destination surface needed
void Surface::blit_surface(SDL_Surface* source, SDL_Rect* srcRect, SDL_Surface* dest, int destX, int destY)
{
	SDL_Rect destRect;	//needed local as this is static fn
	destRect.x = destX;
	destRect.y = destY;
	SDL_BlitSurface( source, srcRect, dest, &destRect );
}

//leave srcRect clip param 0 (or NULL) to blit whole surface or specify srcRect clip to blit
//only part of the source to the destination
void Surface::blit_surface(SDL_Surface* source, SDL_Rect* srcRect, int destX, int destY)
{
	//set up destination rectangle
	_r.x = destX;
	_r.y = destY;
    _r.w = _r.h = 0;	//height/width ignored by SDL_BlitSurface
	//do it
	SDL_BlitSurface( source, srcRect, _surface, &_r );
}


#if 0
// This is a way of telling whether or not to use hardware surfaces
Uint32 FastestFlags(Uint32 flags, int width, int height, int bpp)
{
   const SDL_VideoInfo *info;

   /* Hardware acceleration is only used in fullscreen mode */
   flags |= SDL_FULLSCREEN;

   /* Check for various video capabilities */
   info = SDL_GetVideoInfo();
   if ( info->blit_hw_CC && info->blit_fill ) {
      /* We use accelerated colorkeying and color filling */
      flags |= SDL_HWSURFACE;
   }
   /* If we have enough video memory, and will use accelerated
      blits directly to it, then use page flipping.
    */
   if ( (flags & SDL_HWSURFACE) == SDL_HWSURFACE ) {
      /* Direct hardware blitting without double-buffering
         causes really bad flickering.
       */
      if ( info->video_mem*1024 > (height*width*bpp/8) ) {
         flags |= SDL_DOUBLEBUF;
      } else {
         flags &= ~SDL_HWSURFACE;
      }
   }

   /* Return the flags */
   return(flags);
}
#endif

