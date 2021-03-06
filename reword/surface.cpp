////////////////////////////////////////////////////////////////////
/*

File:			surface.cpp

Class impl:		Surface

Description:	A class to wrap a SDL surface. Screen and Image classes are
				derived from this.

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007


History:		Version	Date		Change
				-------	----------	--------------------------------
				0.7		02.01.17	Moved to SDL2

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


#include <SDL_image.h>	//for IMG_ functions

#include <iostream>

#include "global.h"
#include "surface.h"
#include "utils.h"
#include "platform.h"
#include "locator.h"

Surface::Surface() : _surface(nullptr)
{
}

Surface::Surface(SDL_Surface *s) : _surface(nullptr)
{
    setSurface(s);
	_surface = s;
	s->refcount++;
}

Surface::~Surface()
{
	cleanUp();
}

//create a specific size surface, used mainly by Image class
bool Surface::create(Uint32 w, Uint32 h, int iAlpha /*=-1*/)
{
	cleanUp();	//destroy any existing surface

    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

	_surface = SDL_CreateRGBSurface(
                          0,            //unused in SDL2
                          w, h,
                          SCREEN_BPP,   //from platform.h
                          rmask, gmask, bmask, amask);  //used if > 8 bits

    /* or using the default masks for the depth: */
    //_surface = SDL_CreateRGBSurface(0,w,h,32,0,0,0,0);

    SDL_SetSurfaceBlendMode(_surface, SDL_BLENDMODE_BLEND);  //this is default anyway

	return _surface != nullptr;
}

//just load the image named
bool Surface::load(const std::string &fileName)
{
    cleanUp();
    //_surface = SDL_LoadBMP( fileName.c_str() );	//using SDL dll

    //if fileName does not include a path prefix (has / or \ included in filename)
    //then add RES_IMAGES prefix automatically. So callers can just use base filename or
    //add own explicit path.
    const bool bHasPath = (fileName.find_first_of("\\/") != std::string::npos);
    if (bHasPath)
        _surface = IMG_Load(fileName.c_str());		//using SDL_Image dll (png, jpg etc)
    else
        _surface = IMG_Load((RES_IMAGES + fileName).c_str());		//using SDL_Image dll (png, jpg etc)
	if (nullptr == _surface)
	{
		std::cerr << "Failed to load image " << (bHasPath?"":(RES_IMAGES).c_str()) << fileName << ". Cannot start." << std::endl;
		std::cerr << "SDL_Error = " << SDL_GetError() << std::endl;
		return false;
	}

    SDL_SetSurfaceBlendMode(_surface, SDL_BLENDMODE_BLEND);  //this is default anyway

    return true;
}

void Surface::setTransparentColour(SDL_Color cAlphaKey)
{
    if (_surface == nullptr) return;

    Uint32 key = SDL_MapRGB(_surface->format, cAlphaKey.r,cAlphaKey.g,cAlphaKey.b);
    SDL_SetColorKey(_surface, 1, key);
}

void Surface::setTransparentColour()
{
    if (_surface == nullptr) return;

    /* Set transparent pixel as that of the pixel at (0,0) */

    if (_surface->format->palette) {
        SDL_SetColorKey(_surface, 1, *(Uint8 *) _surface->pixels);
    } else {
        switch (_surface->format->BitsPerPixel) {
        case 15:
            SDL_SetColorKey(_surface, 1, (*(Uint16 *) _surface->pixels) & 0x00007FFF);
            break;
        case 16:
            SDL_SetColorKey(_surface, 1, *(Uint16 *) _surface->pixels);
            break;
        case 24:
            SDL_SetColorKey(_surface, 1, (*(Uint32 *) _surface->pixels) & 0x00FFFFFF);
            break;
        case 32:
            SDL_SetColorKey(_surface, 1, *(Uint32 *) _surface->pixels);
            break;
        }
    }
}

void Surface::setAlphaTransparency(Uint8 iAlpha)
{
    if (_surface == nullptr) return;
    SDL_SetSurfaceAlphaMod(_surface, iAlpha);
}

/*
bool Surface::initSurface(int iAlpha)
{
//	if( newSurface != nullptr )
//	{
//		//Create an optimized image
//		if (newSurface->format->Amask && iAlpha >= 0)
//		{
//			 //per pixel - iAlpha: 0=transparent, 255 = full opacity
//			SDL_SetAlpha(newSurface, SDL_SRCALPHA, iAlpha);
//			_surface = SDL_DisplayFormatAlpha( newSurface );
//		}
//		else
//		{
//			//set the transparent pixel - doesnt affect per pixel alpha surfaces
//			Uint32 key = SDL_MapRGB(newSurface->format, 255,0,255); //putrid purple
//			SDL_SetColorKey(newSurface, SDL_TRUE, key);
//			if (iAlpha >= 0)
//				SDL_SetAlpha(newSurface, SDL_SRCALPHA, iAlpha); //per surface transparency only
//			else
//				SDL_SetAlpha(newSurface, 0, 0); //no transparency
//			_surface = SDL_DisplayFormat( newSurface );
//		}
//
//		//Free the old image
//		SDL_FreeSurface( newSurface );

		return true;
//	}
//	return false;
}
*/

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
	_surface = nullptr;
}

void Surface::setSurface(SDL_Surface *s)
{
	if (s == nullptr) return;
	cleanUp();
	_surface = s;
	s->refcount++;
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
Uint32 Surface::width(void) const
{
	if (!_surface) return 0;
    return _surface->w;
}

// Get height
Uint32 Surface::height(void) const
{
	if (!_surface) return 0;
    return _surface->h;
}


Texture::Texture() :
    _texture(nullptr), _width(0), _height(0)
{
}
Texture::Texture(Surface &surface) :
    _texture(nullptr), _width(0), _height(0)
{
    createFrom(surface);
}
void Texture::createFrom(Surface &surface)
{
//    SDL_SetSurfaceBlendMode(surface.surface(), SDL_BLENDMODE_BLEND);  //this is default anyway

    _texture = SDL_CreateTextureFromSurface(Locator::screen().renderer(), surface.surface());

    SDL_SetSurfaceBlendMode(surface.surface(), SDL_BLENDMODE_BLEND);  //this is default anyway

    Uint32 format;
    int access, w, h;
    SDL_QueryTexture(_texture, &format, &access, &w, &h);
    _width = w;
    _height = h;
}
void Texture::cleanup()
{
    if (_texture)
    {
        SDL_DestroyTexture(_texture);
        _texture = nullptr;
    }
}
Texture::~Texture()
{
    cleanup();
}



