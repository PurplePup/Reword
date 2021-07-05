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
#include "platform.h"
#include "fontttf.h"

#include <iostream>
#include <assert.h>
#include <memory>

#include "./tinyxml/tinyxml.h"

#define XMIDPOSITION    (-999)

FontTTF::FontTTF() :
    _font(nullptr), _size(0), _init(false), _height(0),
    _bFastTTF(false), _bFastBMP(false)
{
	_fontColour = BLACK_COLOUR;
	_shadowColour = WHITE_COLOUR;
}

FontTTF::FontTTF(const std::string &fileName, int fontSize, const std::string &desc) :
    _font(nullptr), _size(fontSize), _init(false), _height(0),
    _bFastTTF(false), _bFastBMP(false)
{
	loadTTF(fileName, fontSize, desc);
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
    _bFastBMP = _bFastTTF = false;
    _fastTex.reset();
	_init = false;
}

bool FontTTF::loadTTF(const std::string &fileName, int fontSize, const std::string &desc)
{
	cleanUp();
	_size = fontSize;
	_description += desc;   //APPEND

	_font = TTF_OpenFont( fileName.c_str(), fontSize );
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

	_fileName = fileName;
	_faceName = fileName;   //same as filename for TTF load

	_init = (_font != nullptr);

//	if (_init)
//        convertToFastTexture();

	return _init;
}

bool FontTTF::convertToFastTexture()
{
//    TTF_SetFontHinting(_font, TTF_HINTING_NONE);    //reduces glitching
    TTF_SetFontHinting(_font, TTF_HINTING_MONO);    //reduces glitching

    //create a texture containing each char
    std::string chrs;
    int prevStart(0);
    for (int i = 0; i < 255; ++i)
    {
//        const char c = (i<32 ||     //unprintable below space
//                         (i==92) || (i==94) || (i==95)  //cause glitches in glyphs
//                        || i>122)   //more glitches
//                        ? 'X' : (char)i;  //make duff chars ok, else actual char
        const char c = (i<32     //unprintable below space
                         || (i>=33 && i<=47)  //cause glitches in glyphs
                         || (i>=58 && i<=63)  //cause glitches in glyphs
                         || (i>=91 && i<=96)  //cause glitches in glyphs
                         || i>122)   //more glitches
                        ? '-' : (char)i;  //make duff chars ok, else actual char

        //save for conversion into texture
        chrs += c;

        //get individual char width and create width and offset for rendering
        char character[2] = { c, 0x00 };

        int w, h;
        int result = TTF_SizeText(_font, &character[0], &w, &h);

        const int len = calc_text_length(&character[0]);
        SFastWidths widths = {prevStart, len, 0, 0, 0, 0, 0};
        _fastWidths.push_back(widths);
        prevStart += len;
    }

    SDL_Surface *text = TTF_RenderText_Blended( _font, &chrs[0], _fontColour );
    if (text)
    {
//        if (XMIDPOSITION == x) x = (s->width() - text->w ) / 2;
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
    return _bFastTTF = true;
}

bool FontTTF::loadBMP(const std::string &fontInfoName, const std::string &desc)
{
    if (fontInfoName.length() == 0) return false;

    std::unique_ptr<TiXmlDocument> doc(new TiXmlDocument(fontInfoName));
    if (!doc->LoadFile())
    {

        return false;
    }

    /* example angelcode BMFont .fmt file xml format
    <font>
      <info face="FreeSans" size="14" bold="1" italic="0" charset="" unicode="" stretchH="100" smooth="1" aa="1" padding="2,2,2,2" spacing="0,0" outline="0"/>
      <common lineHeight="22" base="16" scaleW="880" scaleH="20" pages="1" packed="0"/>
      <pages>
        <page id="0" file="font.png"/>
      </pages>
      <chars count="80">
        <char id="97" x="2" y="2" width="9" height="10" xoffset="0" yoffset="3" xadvance="8" page="0" chnl="15"/>
        <char id="98" x="13" y="2" width="10" height="13" xoffset="1" yoffset="0" xadvance="9" page="0" chnl="15"/>
        ...
      </chars>
    </font>
    */

   	TiXmlElement *root = doc->FirstChildElement("font");
	if (root)
	{
	    std::string strVal;
		TiXmlElement * e = root->FirstChildElement("info");
		if (!e) return false;
        _faceName = e->Attribute("face");
        e->Attribute("size", &_size);

        e = root->FirstChildElement("common");
		if (!e) return false;
        strVal = e->Attribute("lineHeight", &_lineHeight);
        _height = _lineHeight;

        e = root->FirstChildElement("pages");
		if (!e) return false;
        auto ee = e->FirstChildElement("page");
        if (!ee) return false;
        _fileName = ee->Attribute("file");

        e = root->FirstChildElement("chars");
		if (!e) return false;
        _fastWidths.clear();
        SFastWidths widths = {0, 0, 0, 0, 0, 0, 0};
        for (int i = 0; i < 255; ++i)
        {
            _fastWidths.push_back(widths);
        }
        int pos(0), val(0);
        for(TiXmlElement* e1 = e->FirstChildElement("char"); e1 != nullptr; e1 = e1->NextSiblingElement("char"))
        {
            e1->Attribute("id", &pos);
            if (pos < 0 || pos > 255)
            {
                std::cout << "fontBmp " << _fileName << "(" << _faceName << ") id:" << pos << " invalild" << std::endl;
                continue;
            }
            e1->Attribute("x", &val);
            _fastWidths[pos].startX = val;
            e1->Attribute("y", &val);
            _fastWidths[pos].startY = val;
            e1->Attribute("width", &val);
            _fastWidths[pos].width = val;
            e1->Attribute("height", &val);
            _fastWidths[pos].height = val;
            e1->Attribute("xoffset", &val);
            _fastWidths[pos].offsetX = val;
            e1->Attribute("yoffset", &val);
            _fastWidths[pos].offsetY = val;
            e1->Attribute("xadvance", &val);
            _fastWidths[pos].advanceX = val;

        }
	}

    //load font image now
    Surface surface;
    surface.load(RES_FONTS + _fileName);
    _fastTex = std::unique_ptr<Texture>(new Texture(surface));

    return _init = _bFastBMP = true;
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
        if (_bFastBMP || _bFastTTF)
        {
            //must calc size ourselves
            int nextX = xOffset;
            char * pos = const_cast<char*>(textstr);
            while (*pos)
            {
                nextX += _fastWidths[(*pos)].advanceX;
                ++pos;
            }
            r._min.x = xOffset;
            r._min.y = yOffset;
            r._max.x = xOffset + nextX;
            r._max.y = yOffset + _height;
        }
        else
        {
            //FontTTF size function (even if _bFastTTF as font is loaded first)
            int w, h;
            int result = TTF_SizeText(_font, textstr, &w, &h);
            if (result != -1)
            {
                r._min = Point(xOffset, yOffset);
                r._max = r._min.add(Point(w+(bShadow?1:0), h+(bShadow?1:0)));
            }
            return r;
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
	if (textstr != nullptr && (_bFastTTF || _bFastBMP)) //either fast texture mode as long as we don't use TTF functions
    {
        SDL_SetTextureColorMod(_fastTex->texture_sdl(), (Uint8)textColour.r, (Uint8)textColour.g, (Uint8)textColour.b);

		if (XMIDPOSITION == x)
        {
            int w = 0;
            char * pos = const_cast<char*>(textstr);
            while (*pos)
            {
                w += _fastWidths[(*pos)].advanceX;
                ++pos;
            }
            x = (s->width() - w ) / 2;
        }
        int nextX = x;
        char * pos = const_cast<char*>(textstr);
        while (*pos)
        {
            //blit each char
            const SFastWidths *w = &_fastWidths[(*pos)];
            SDL_Rect srcLetter = {w->startX, w->startY, w->width, w->height};
			s->blit(_fastTex->texture_sdl(), &srcLetter, nextX + w->offsetX, y + w->offsetY);
			nextX += w->advanceX;
			++pos;
        }
        Rect r(x, y, x + nextX, y + _height);
        return r;
    }

	Rect r(0, 0, 0, 0);

	if (bShadow)
	{
		SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, _shadowColour );
		if (text)
		{
			if (XMIDPOSITION == x) x = (s->width() - text->w ) / 2;

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
		if (XMIDPOSITION == x) x = (s->width() - text->w ) / 2;

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
//		if (XMIDPOSITION == x) x = (s->width() - text->w ) / 2;
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
	return put_text(s, XMIDPOSITION, y, textstr, textColour, bShadow);
}

//right justify to edge of screen
Rect FontTTF::put_text_right(Screen *s, int xDelta, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	const int len = calc_text_length(textstr);
	return put_text(s, s->width() - len - xDelta, y, textstr, textColour, bShadow);
}

Rect FontTTF::put_text_mid(Screen *s, int xMid, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
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
	return put_text(s, XMIDPOSITION, y, _buffer, textColor, bShadow);
}

Rect FontTTF::put_number_right(Screen *s, int xDelta, int y, int number, const char *format, const SDL_Color &textColour, bool bShadow /*= false*/)
{
    snprintf(_buffer, sizeof(_buffer), format, number);
	const int len = calc_text_length(_buffer);
	return put_text(s, s->width() - len - xDelta, y, _buffer, textColour, bShadow);
}

//place number mid point at xMid position i.e "NNNxNNN" gixing x as the mid point
Rect FontTTF::put_number_mid(Screen *s, int xMid, int y, int number, const char *format, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	const int len = calc_text_length(_buffer);
	return put_text(s, xMid - (len / 2), y, _buffer, textColour, bShadow);
}


// Surface functions ////////////////////////////


#if 0

//output a text string to the destination surface
//If shadow selected, then draw in the shadow colour first offset by 1, then the text in the actual position
Rect FontTTF::put_text(Texture *t, int x, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{

//	int textW(0);
//	SDL_Surface *text = TTF_RenderText_Solid( _font, _buffer, textColour );
//	if (text)
//	{
//	    textW = text->w;
//	    SDL_FreeSurface(text);
//	}
//


    if (_bFastBMP || _bFastTTF)
    {
//TODO

	//	SDL_SetTextureColorMod(_fastTex->texture_sdl(), (Uint8)textColour.r, (Uint8)textColour.g, (Uint8)textColour.b);

		Uint32 format;
		int access, txw, txh;
		SDL_QueryTexture(t->texture_sdl(), &format, &access, &txw, &txh);

		int nextX = x;
		if (XMIDPOSITION == x)
		{
			int w = 0;
			char * pos = const_cast<char*>(textstr);
			while (*pos)
			{
				w += _fastWidths[(*pos)].advanceX;
				++pos;
			}
			nextX = (txw - w) / 2;
		}
		char * pos = const_cast<char*>(textstr);
		while (*pos)
		{
			//blit each char
			const SFastWidths *w = &_fastWidths[(*pos)];
			SDL_Rect srcLetter = { w->startX, w->startY, w->width, w->height };

			ppg::blit_surface(_fastTex->texture_sdl(), 
			s->blit( _fastTex->texture_sdl(), &srcLetter, nextX + w->offsetX, y + w->offsetY);
			nextX += w->advanceX;
			++pos;
		}
		Rect r(x, y, x + nextX, y + _height);
		return r;

//		SDL_RenderCopy( _renderer, srcTex, srcRect, &destRect);
//      return r;
    }

	Rect r(0, 0, 0, 0);

	if (bShadow)
	{
		SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, _shadowColour );
		if (text)
		{
			if (XMIDPOSITION == x) x = (s->width() - text->w ) / 2;

			r._min = Point(x, y);
			r._max = r._min.add(Point(text->w+1, text->h+1));

			ppg::blit_surface(text, nullptr, s->surface(), x+1, y+1);

			SDL_FreeSurface(text);
		}
	}

	SDL_Surface *text = TTF_RenderText_Blended( _font, textstr, textColour );
	if (text)
	{
		if (XMIDPOSITION == x) x = (s->width() - text->w ) / 2;

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
	return put_text(s, XMIDPOSITION, y, textstr, textColour, bShadow);
}

//right justify to edge of surface xDelta given
Rect FontTTF::put_text_right(Surface *s, int xDelta, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	const int len = calc_text_length(textstr);
	return put_text(s, s->width() - len - xDelta, y, textstr, textColour, bShadow);
}

Rect FontTTF::put_text_mid(Surface *s, int xMid, int y, const char *textstr, const SDL_Color &textColour, bool bShadow /*= false*/)
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
	return put_text(s, XMIDPOSITION, y, _buffer, textColor, bShadow);
}

Rect FontTTF::put_number_right(Surface *s, int xDelta, int y, int number, const char *format, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	const int len = calc_text_length(_buffer);
	return put_text(s, s->width() - len - xDelta, y, _buffer, textColour, bShadow);
}

//place number mid point at xMid position i.e "NNNxNNN" gixing x as the mid point
Rect FontTTF::put_number_mid(Surface *s, int xMid, int y, int number, const char *format, const SDL_Color &textColour, bool bShadow /*= false*/)
{
	snprintf(_buffer, sizeof(_buffer), format, number);
	const int len = calc_text_length(_buffer);
	return put_text(s, xMid - (len / 2), y, _buffer, textColour, bShadow);
}

#endif 

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

