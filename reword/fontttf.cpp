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
#include <assert.h>
#include <memory>


FontTTF::FontTTF() : _font(nullptr), _size(0), _init(false), _height(0), _bFastTex(false)
{
	_fontColour = BLACK_COLOUR;
	_shadowColour = WHITE_COLOUR;
}

FontTTF::FontTTF(const std::string &fileName, int size, const std::string &desc) :
    _font(nullptr), _size(size), _init(false), _height(0), _bFastTex(false)
{
	load(fileName, size, desc);
	memset(&_shadowColour, 0, sizeof(SDL_Color));	//black {0x00,0x00,0x00,0},
}

FontTTF::~FontTTF()
{
	cleanUp();
}

void FontTTF::cleanUp()
{
	if (_font) TTF_CloseFont(_font);
	_font = nullptr;
    _size = 0;
    _height = 0;
	_init = false;
}

bool FontTTF::load(std::string fileName, int size, const std::string &desc)
{
	cleanUp();
	_size = size;
	_description += desc;   //APPEND

	_font = TTF_OpenFont( fileName.c_str(), size );
	if (nullptr == _font)
		std::cerr << "Failed to load font " << fileName << ". Cannot start." << std::endl;
	else
    {
        //_height = TTF_FontAscent(_font);    //height above baseline
		_height = TTF_FontHeight(_font);    //full height of font
        /*
		int minx,maxx,miny,maxy,advance;
		if(TTF_GlyphMetrics(_font,'A',&minx,&maxx,&miny,&maxy,&advance)==-1)
			return 0;
		_height =  maxy;// - miny;
        */
    }

	_filename = fileName;

	_init = (_font != nullptr);
	return _init;
}

bool FontTTF::convertToFastTexture(Screen *s)
{
    TTF_SetFontHinting(_font, TTF_HINTING_NONE);    //reduces glitching

    //create a texture containing each char
    std::vector<char> chrs;
    int prevStart(0);
    for (int i = 0; i < 255; ++i)
    {
        const char c = (i<32 ||     //unprintable below space
                         (i==92) || (i==94) || (i==95)  //cause glitches in glyphs
                        || i>122)   //more glitches
                        ? 'X' : (char)i;  //make duff chars ok, else actual char

        //save for conversion into texture
        chrs.push_back(c);

        //get individual char width and create width and offset for rendering
        std::string character;
        character += c;

        int w, h;
        int result = TTF_SizeText(_font, character.c_str(), &w, &h);

        const int len = calc_text_length(character.c_str());
        SFastWidths widths = {prevStart, len};
        _fastWidths.push_back(widths);
        prevStart += len;
    }
    chrs.push_back(0);

    SDL_Surface *text = TTF_RenderText_Blended( _font, &chrs[0], _fontColour );
    if (text)
    {
//        if (-999 == x) x = (s->width() - text->w ) / 2;
//
//        if(!bShadow){
//            r._min = Point(x, y);
//            r._max = r._min.add(Point(text->w, text->h));
//        }

        Surface surface(text);
        _fastTex = std::unique_ptr<Texture>(new Texture(surface));

        SDL_FreeSurface(text);
    }

    //set flag to indicate use of in put_text() functions
    return _bFastTex = true;
}

//set the font shadow (to something other than the default black)
void FontTTF::setShadowColour(SDL_Color &c)
{
	_shadowColour = c;
}

Rect FontTTF::calc_text_metrics(const char *textstr, bool bShadow /*= false*/, int xOffset, int yOffset) const
{
	Rect r(0, 0, 0, 0);
    if (textstr != nullptr)
    {
        int w, h;
        int result = TTF_SizeText(_font, textstr, &w, &h);
        if (result != -1)
        {
            r._min = Point(xOffset, yOffset);
            r._max = r._min.add(Point(w+(bShadow?1:0), h+(bShadow?1:0)));
        }
    }
	return r;
}

//simpler fn to just return length of string in pixels
int FontTTF::calc_text_length(const char *textstr, bool bShadow /*= false*/) const
{
	Rect r = calc_text_metrics(textstr, bShadow, 0, 0);
	return r._max.x;
}

//output a text string to the destination surface
//If shadow selected, then draw in the shadow colour first offset by 1, then the text in the actual position
Rect FontTTF::put_text(Screen *s, int x, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	if (_bFastTex && textstr != nullptr)
    {
        int nextX = x;
        char * pos = const_cast<char*>(textstr);
        while (*pos)
        {
            assert(*pos > 0 && *pos < 255);
            //blit each char
            const int start = _fastWidths[(*pos)].start;
            const int width = _fastWidths[(*pos)].width;
            SDL_Rect r2 = {start, 0, width, height()};
			s->blit(_fastTex->texture_sdl(), &r2, nextX, y);
			nextX += width;
			++pos;
        }
        Rect r(x, y, x + nextX, y + height());
        return r;
    }

	Rect r(0, 0, 0, 0);

	if (bShadow)
	{
		SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, _shadowColour );
		if (text)
		{
			if (-999 == x) x = (s->width() - text->w ) / 2;

			//ppg::blit_surface(text, nullptr, s->surface(), x+1, y+1);
			r._min = Point(x, y);
			r._max = r._min.add(Point(text->w+1, text->h+1));

            SDL_Texture * ptex = SDL_CreateTextureFromSurface(s->renderer(), text);
            SDL_Rect r2 = r.toSDL();
			s->blit(ptex, &r2, x+1, y+1);

            SDL_DestroyTexture(ptex);
			SDL_FreeSurface(text);
		}
	}

	SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, textColour );
	if (text)
	{
		if (-999 == x) x = (s->width() - text->w ) / 2;

		if(!bShadow){
			r._min = Point(x, y);
			r._max = r._min.add(Point(text->w, text->h));
		}

        SDL_Texture * ptex = SDL_CreateTextureFromSurface(s->renderer(), text);
        s->blit(ptex, nullptr, x, y);

        SDL_DestroyTexture(ptex);
		SDL_FreeSurface(text);
	}

//	SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, textColour );
//	if (text)
//	{
//		if (-999 == x) x = (s->width() - text->w ) / 2;
//
//		//ppg::blit_surface(text, nullptr, s->surface(), x, y);
//		if(!bShadow){
//			r._min = Point(x, y);
//			r._max = r._min.add(Point(text->w, text->h));
//		}
//
//      SDL_Texture * ptex = SDL_CreateTextureFromSurface(s->renderer(), text);
//      s->blit(ptex, nullptr, x, y);
//
//      SDL_DestroyTexture(ptex);
//		SDL_FreeSurface(text);
//	}
//
	return r;
}

//centered text
Rect FontTTF::put_text(Screen *s, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	return put_text(s, -999, y, textstr, textColour, bShadow);
}

//right justify to edge of screen
Rect FontTTF::put_text_right(Screen *s, int y, int xDelta, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
    int textW(0);
	SDL_Surface *text = TTF_RenderText_Solid( _font, textstr, textColour );
	if (text)
	{
        textW = text->w;
        SDL_FreeSurface(text);
	}
	return put_text(s, s->width() - textW - xDelta, y, textstr, textColour, bShadow);
}

Rect FontTTF::put_text_mid(Screen *s, int y, int xMid, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	const int len = calc_text_length(textstr);
	return put_text(s, xMid - (len / 2), y, textstr, textColour, bShadow);
}

//output a integer number string to the destination surface
Rect FontTTF::put_number(Screen *s, int x, int y, int number, const char *format, const SDL_Color &textColor, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	return put_text(s, x, y, _buffer, textColor, bShadow);
}

//center a integer number string to the destination surface
Rect FontTTF::put_number(Screen *s, int y, int number, const char *format, const SDL_Color &textColor, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	return put_text(s, -999, y, _buffer, textColor, bShadow);
}

Rect FontTTF::put_number_right(Screen *s, int y, int xDelta, int number, const char *format, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	int textW(0);
	SDL_Surface *text = TTF_RenderText_Solid( _font, _buffer, textColour );
	if (text)
	{
	    textW = text->w;
	    SDL_FreeSurface(text);
	}
	return put_text(s, s->width() - textW - xDelta, y, _buffer, textColour, bShadow);
}

//place number mid point at xMid position i.e "NNNxNNN" gixing x as the mid point
Rect FontTTF::put_number_mid(Screen *s, int y, int xMid, int number, const char *format, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	const int len = calc_text_length(_buffer);
	return put_text(s, xMid - (len / 2), y, _buffer, textColour, bShadow);
}


// Surface functions ////////////////////////////

//output a text string to the destination surface
//If shadow selected, then draw in the shadow colour first offset by 1, then the text in the actual position
Rect FontTTF::put_text(Surface *s, int x, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	Rect r(0, 0, 0, 0);
	if (bShadow)
	{
		SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, _shadowColour );
		if (text)
		{
			if (-999 == x) x = (s->width() - text->w ) / 2;

			r._min = Point(x, y);
			r._max = r._min.add(Point(text->w+1, text->h+1));

			ppg::blit_surface(text, nullptr, s->surface(), x+1, y+1);

			SDL_FreeSurface(text);
		}
	}

	SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, textColour );
	if (text)
	{
		if (-999 == x) x = (s->width() - text->w ) / 2;

		if(!bShadow){
			r._min = Point(x, y);
			r._max = r._min.add(Point(text->w, text->h));
		}

		ppg::blit_surface(text, nullptr, s->surface(), x, y);

		SDL_FreeSurface(text);
	}

	return r;
}

//centered text on surface dest
Rect FontTTF::put_text(Surface *s, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	return put_text(s, -999, y, textstr, textColour, bShadow);
}

//right justify to edge of surface xDelta given
Rect FontTTF::put_text_right(Surface *s, int y, int xDelta, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
    int textW(0);
	SDL_Surface *text = TTF_RenderText_Solid( _font, textstr, textColour );
	if (text)
	{
        textW = text->w;
        SDL_FreeSurface(text);
	}
	return put_text(s, s->width() - textW - xDelta, y, textstr, textColour, bShadow);
}

Rect FontTTF::put_text_mid(Surface *s, int y, int xMid, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	const int len = calc_text_length(textstr);
	return put_text(s, xMid - (len / 2), y, textstr, textColour, bShadow);
}

//output a integer number string to the destination surface
Rect FontTTF::put_number(Surface *s, int x, int y, int number, const char *format, const SDL_Color &textColor, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	return put_text(s, x, y, _buffer, textColor, bShadow);
}

//center a integer number string to the destination surface
Rect FontTTF::put_number(Surface *s, int y, int number, const char *format, const SDL_Color &textColor, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	return put_text(s, -999, y, _buffer, textColor, bShadow);
}

Rect FontTTF::put_number_right(Surface *s, int y, int xDelta, int number, const char *format, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	int textW(0);
	SDL_Surface *text = TTF_RenderText_Solid( _font, _buffer, textColour );
	if (text)
	{
	    textW = text->w;
	    SDL_FreeSurface(text);
	}
	return put_text(s, s->width() - textW - xDelta, y, _buffer, textColour, bShadow);
}

//place number mid point at xMid position i.e "NNNxNNN" gixing x as the mid point
Rect FontTTF::put_number_mid(Surface *s, int y, int xMid, int number, const char *format, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	const int len = calc_text_length(_buffer);
	return put_text(s, xMid - (len / 2), y, _buffer, textColour, bShadow);
}

// end surface render functions /////////////////////////////

//Helper for FontCache to return a font rendered surface
Surface * FontTTF::make_surface(const char *textstr, const SDL_Color &textColour, bool bShadow)
{
    if (!_init) return nullptr;  //fail

    Rect r = calc_text_metrics(textstr, bShadow);
    Surface * s = new Surface();
    s->create(r.width()+(bShadow?1:0), r.height()+(bShadow?1:0));

    if (bShadow)
    {
        SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, _shadowColour );
        if (text)
        {
			ppg::blit_surface(text, nullptr, s->surface(), 1, 1);
            SDL_FreeSurface(text);
        }
    }

    SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, textColour );
    if (text)
    {
        ppg::blit_surface(text, nullptr, s->surface(), 0, 0);
        SDL_FreeSurface(text);
    }

    return s;
}

//pass in index 0 to allocate new index or a valid number > 0 to set that index explicitly
Uint32 FontCache::add(FontTTF &ttf, Uint32 index, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
    Surface * s = ttf.make_surface(textstr, textColour, bShadow);
    if (s == nullptr)
    {
        std::cerr << "FontCache add(" << textstr << ") failed to create surface" << std::endl;
        return 0; //fail
    }

    //add to cache
    const Uint32 idxNew = (index == 0) ? (Uint32)_imageMap.size()+1 : index;    //start at 1 (return 0 for error)

    auto prev = get(idxNew);
    if (prev != nullptr)
    {
        std::cerr << "FontCache add(" << idxNew << ") already exists !!" << std::endl;
        return 0;
    }

    _imageMap[idxNew] = tSharedImage(new Image(*s));    // = std::move(tUniqueImage(new Image(*s)));

#ifdef DEBUG
//    std::cout << "FontCache::add ( idx:" << idxNew << " fontname:" << ttf.filename() <<
//        " fontsize:" << ttf.size() << " text:" << textstr << " size:" <<
//        s->width() << "x" << s->height() << " )" << std::endl;
#endif

    delete s;

	return idxNew;   //up to caller to remember cache position if pass in index=0
}

Image * FontCache::get(Uint32 index)
{
    auto it = _imageMap.find(index);
    if (it == _imageMap.end())
        return nullptr; //not found

    return it->second.get();
}

