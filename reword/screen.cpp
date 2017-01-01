////////////////////////////////////////////////////////////////////
/*

File:			screen.cpp

Class impl:		Screen

Description:	A class to manage drawing to the screen

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007
                08 Oct 2013     - rewite to SDL2

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

#include "global.h"
#include "screen.h"
#include "platform.h"

#include <cassert>

int Screen::_height = 0;
int Screen::_width = 0;

Screen::Screen() :
    _window(nullptr), _renderer(nullptr), _texture(nullptr),
    _init(false)
{
}

// Construct 16 bit colour screen of given size
Screen::Screen(int w, int h, const std::string &strTitle) :
    _window(nullptr), _renderer(nullptr), _texture(nullptr),
    _init(false)
{
	assert(!(w<320 || h<240));	//reasonable minimum for existing game graphics etc

	_window = SDL_CreateWindow(strTitle.c_str(),
                            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
                            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
                            w, h,
                            SDL_WINDOW_SHOWN);

	if ( _window != nullptr )
	{
#if ((!defined(GP2X) && !defined(PANDORA)))
        SDL_SetWindowTitle(_window, strTitle.c_str());		//windowed caption
        std::cout << "Using window, Caption " << strTitle << std::endl;
#endif

        //create screen renderer
        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
        if (_renderer == nullptr)
		{
			setLastError("Unable to create accelerated renderer - trying software renderer");
	        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_SOFTWARE);
		}

        if (_renderer != nullptr)
        {
            //create texture (in GPU mem) to use as screen to 'flip'
            _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING, w, h);
            if(_texture != nullptr)
            {
                _width = w;
                _height = h;
                _init = true;
            }
            else
            {
                setLastError("Unable to create screen texture");
            }
        }
        else
        {
            setLastError("Unable to create screen renderer");
        }
	}
	else
	{
		setLastError("Unable to set video resolution");
	}
}

Screen::~Screen()
{
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
}

// Lock screen
void Screen::lock(void)
{
//    if (SDL_MUSTLOCK(_window))
//		if (SDL_LockSurface(_window) < 0)
//		    return;
}

// Unlock screen
void Screen::unlock(void)
{
//    if (SDL_MUSTLOCK(_window))
//		SDL_UnlockSurface(_window);
}

// Update whole screen
void Screen::update(void)
{
//    SDL_SetRenderTarget()

    //SDL_RenderClear(_renderer);
    //SDL_RenderCopy(_renderer, _texture, nullptr, nullptr);
	SDL_RenderPresent(_renderer);
}

void Screen::clear()
{
    SDL_SetRenderDrawColor(_renderer, 0x00, 0x00, 0x00, 0xFF);  //black
    SDL_RenderClear(_renderer);
}

// Draw a filled/solid rectangle
void Screen::drawSolidRect (int x, int y, int w, int h, const SDL_Color& c)
{
    SDL_Rect r = { x, y, w, h };
    SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(_renderer, &r);
}

// Draw a filled/solid alpha blended (transparent) rectangle
void Screen::drawSolidRectA(int x, int y, int w, int h, const SDL_Color& c, int iAlpha)
{
    SDL_Rect r = { x, y, w, h };
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(_renderer, &r);
}

void Screen::putPixel(int x, int y, Uint32 colour)
{
    assert(0);  //fn not implemented

//	// Put pixel of colour c at x,y
//	// If colour is NONE - no pixel is ploted
//    if (x >=0 && x < s->w && y >=0 && y < s->h) // && c != Colour::NONE)
//    {
//		unsigned short* dst = static_cast<unsigned short*>(s->pixels);
//		dst[y * s->pitch/sizeof(unsigned short) + x] = (unsigned short)colour;	//##fudge!!
//    }
}

//function to blit a SDL_Texture to screen
Rect Screen::blit(SDL_Texture* srcTex, SDL_Rect* srcRect, int destX, int destY)
{
    SDL_Rect destRect;
    destRect.x = destX;
    destRect.y = destY;
    if (srcRect == nullptr)
    {
        Uint32 format;
        int access, w, h;
        SDL_QueryTexture(srcTex, &format, &access, &w, &h);
        destRect.w = w;
        destRect.h = h;
    }
    else
    {
        destRect.w = srcRect->w;
        destRect.h = srcRect->h;
    }
	SDL_RenderCopy(_renderer, srcTex, srcRect, &destRect);
	return Rect(destRect);
}

//function to blit a Texture to screen
Rect Screen::blit(Texture* srcTex, SDL_Rect* srcRect, int destX, int destY)
{
//	SDL_Rect destRect = { destX, destY, srcTex->width(), srcTex->height() };
//	SDL_RenderCopy(_renderer, srcTex->texture(), srcRect, &destRect);
//    blit(srcTex->texture(), srcRect, destX, destY);

    SDL_Rect destRect;
    destRect.x = destX;
    destRect.y = destY;
    if (srcRect == nullptr)
    {
        destRect.w = srcTex->width();
        destRect.h = srcTex->height();
    }
    else
    {
        destRect.w = srcRect->w;
        destRect.h = srcRect->h;
    }
	SDL_RenderCopy(_renderer, srcTex->texture_sdl(), srcRect, &destRect);
	return Rect(destRect);
}

//blit texture using deltaX to center texture on (mainly used for text positioning)
Rect Screen::blit_mid(Texture* srcTex, SDL_Rect* srcRect, int destDeltaX, int destY, bool bAbsolute)
{
	const int newX = bAbsolute ? //center on absolute x position given, else use screen mid point
               destDeltaX - (srcTex->width()/2) : (width()/2) - destDeltaX - (srcTex->width()/2);
    return blit(srcTex, srcRect, newX, destY);
}

//blit texture to right side of screen using deltaX as rightmost edge, so texture blitted
//back from that point not from that point forward.
Rect Screen::blit_right(Texture* srcTex, SDL_Rect* srcRect, int deltaX, int destY)
{
	SDL_Rect destRect = { width() - deltaX - srcTex->width(), destY, srcTex->width(), srcTex->height() };
	SDL_RenderCopy(_renderer, srcTex->texture_sdl(), srcRect, &destRect);
	return Rect(destRect);
}

