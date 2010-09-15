////////////////////////////////////////////////////////////////////
/*

File:			fontttf.cpp

Class impl:		FontTTF

Description:	A class to wrap all font handling, using the SDL_TTF dll

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.5		18.06.2008	Added touch support

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
#include "fontttf.h"

#include <iostream>


FontTTF::FontTTF() : _font(0), _init(false), _height(0)
{
	_shadow = BLACK_COLOUR;
}

FontTTF::FontTTF(std::string fileName, int size) : _font(0), _init(false), _height(0)
{
	load(fileName, size);
	memset(&_shadow, 0, sizeof(SDL_Color));	//black {0x00,0x00,0x00,0},
}

FontTTF::~FontTTF()
{
	cleanUp();
}

void FontTTF::cleanUp()
{
	if (_font) TTF_CloseFont(_font);
	_font = 0;
	_init = false;
}

bool FontTTF::load(std::string fileName, int size)
{
	cleanUp();
	_font = TTF_OpenFont( fileName.c_str(), size );
	if (NULL == _font)
		std::cerr << "Failed to load font " << fileName << ". Cannot start." << std::endl;
	else
//		_height = size;	//calc_text_metrics("X")._y;
//		_height = calc_text_metrics("A")._max._y;
//		_height = TTF_FontHeight(_font);
		_height = TTF_FontAscent(_font);
	/*
		int minx,maxx,miny,maxy,advance;
		if(TTF_GlyphMetrics(_font,'A',&minx,&maxx,&miny,&maxy,&advance)==-1)
			return 0;
		_height =  maxy;// - miny;
	*/
	_init = (_font != NULL);
	return _init;
}

//output a text string to the destination surface
//If shadow selected, then draw in the shadow colour first offset by 1, then the text in the actual position
Rect FontTTF::put_text(Surface *s, int x, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	Rect r(0, 0, 0, 0);
	if (bShadow)
	{
		SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, _shadow ); 
		if (text != NULL)
		{
			if (-999 == x) x = (s->surface()->w - text->w ) / 2;
			s->blit_surface(text, 0, x+1, y+1);
			r._min = Point(x, y);
			r._max = r._min.add(Point(text->w+1, text->h+1));
			SDL_FreeSurface(text);
		}
	}

	SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, textColour ); 
	if (text != NULL)
	{
		if (-999 == x) x = (s->surface()->w - text->w ) / 2;
		s->blit_surface(text, 0, x, y);
		if(!bShadow){	
			r._min = Point(x, y);
			r._max = r._min.add(Point(text->w, text->h));
		}
		SDL_FreeSurface(text);
	}

	return r;
}

//centered text
Rect FontTTF::put_text(Surface *s, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	return put_text(s, -999, y, textstr, textColour, bShadow);
}

Rect FontTTF::put_text_right(Surface *s, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, textColour ); 
	return put_text(s, s->surface()->w - text->w, y, textstr, textColour, bShadow);
}

//output a integer number string to the destination surface
Rect FontTTF::put_number(Surface *s, int x, int y, unsigned int number, const char *format, const SDL_Color &textColor, bool bShadow /*= false*/)
{
	snprintf(_buffer, 100, format, number);
	return put_text(s, x, y, _buffer, textColor, bShadow);
}

//center a integer number string to the destination surface
Rect FontTTF::put_number(Surface *s, int y, unsigned int number, const char *format, const SDL_Color &textColor, bool bShadow /*= false*/)
{
	snprintf(_buffer, 100, format, number);
	return put_text(s, -999, y, _buffer, textColor, bShadow);
}

//set the font shadow (to something other than the default black) 
void FontTTF::setShadowColour(SDL_Color &c)
{
	_shadow = c;
}

Rect FontTTF::calc_text_metrics(const char *textstr, bool bShadow /*= false*/, int x, int y) const
{
	Rect r(0, 0, 0, 0);

//	SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, BLACK_COLOUR );
	SDL_Surface *text = TTF_RenderText_Solid( _font, textstr, BLACK_COLOUR );
	if (text != NULL)
	{
		r._min = Point(x, y);
//		r._max = r._min.add(Point(text->w+(bShadow?1:0), text->h+(bShadow?1:0)));
		r._max = r._min.add(Point(text->w+(bShadow?1:0), height()+(bShadow?1:0)));
		SDL_FreeSurface(text);
	}
	return r;

/*	
	int w, h;
	TTF_SizeText(_font, textstr, &w, &h);
	r._min = Point(x, y);
	r._max = r._min.add(Point(w+(bShadow?1:0), h+(bShadow?1:0)));
	return r;
*/
}

//simpler fn to just return length of string in pixels
int FontTTF::calc_text_length(const char *textstr, bool bShadow /*= false*/) const
{
	Rect r = calc_text_metrics(textstr, bShadow, 0, 0);
	return r._max._x;
}

int FontTTF::height() const	//helper to quickly return font height in pixels
{
	return _height;
}

