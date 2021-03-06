////////////////////////////////////////////////////////////////////
/*

File:			image.cpp

Class impl:		Image

Description:	A class based on the Surface class that encapulates image
				specific functions. Tile handling is included.

				All Image class surfaces use the transparent pixel (255,0,255)
				which is a putrid purple.

				Surface->Image

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

#include "global.h"
#include "image.h"
#include "utils.h"
#include "locator.h"

#include <iostream>

Image::Image() :
	_init(false),
	_tileCount(0), _tileW(0), _tileH(0), _tileWOffset(0), _tileHOffset(0),
	_tileDir(TILE_HORIZ), _ptex(nullptr)
{
	cleanUp();
}

Image::Image(unsigned int w, unsigned int h, Uint32 nTiles /*=1*/, SDL_Color cAlphaKey /*=ALPHA_COLOUR*/, Uint8 iAlpha /* = 255 */ ) :
	_init(false),
	_tileCount(0), _tileW(0), _tileH(0), _tileWOffset(0), _tileHOffset(0),
	_tileDir(TILE_HORIZ), _ptex(nullptr)
{
	cleanUp();
	_init = create(w, h, nTiles, cAlphaKey, iAlpha);	//create surface of required size
}

Image::Image(const std::string &fileName, Uint32 nTiles /*=1*/, SDL_Color cAlphaKey /*=ALPHA_COLOUR*/, Uint8 iAlpha /* = 255 */ ) :
	_init(false),
	_tileCount(0), _tileW(0), _tileH(0), _tileWOffset(0), _tileHOffset(0),
	_tileDir(TILE_HORIZ), _ptex(nullptr)
{
#if _DEBUG
    _dbgName = fileName;
#endif

	cleanUp();
	load(fileName, nTiles, cAlphaKey, iAlpha);
}

//Image::Image(const Image &img) :
//	_init(img._init),
//	_tileCount(0), _tileW(0), _tileH(0), _tileWOffset(0), _tileHOffset(0),
//	_tileDir(TILE_HORIZ), _ptex(nullptr)
//{
//#if _DEBUG
//    _dbgName = img._dbgName;
//#endif
//    *this = img;
//    setTileCount(img.tileCount(), img.tileDir());
//}

//create a texture (Image) from a surface
Image::Image(Surface &surface, Uint32 nTiles /*=1*/, SDL_Color cAlphaKey /*=ALPHA_COLOUR*/, Uint8 iAlpha /* = 255 */ ) :
	_init(false),
	_tileCount(0), _tileW(0), _tileH(0), _tileWOffset(0), _tileHOffset(0),
	_tileDir(TILE_HORIZ), _ptex(nullptr)
{
    initImage(&surface, nTiles, cAlphaKey, iAlpha);
}

Image::~Image()
{
	cleanUp();
}

void Image::cleanUp()
{
//	Surface::cleanUp();	//free surface etc

//	_clip = 0;	//use whole image (used in blit(Image*) )

//    if (_ptex != nullptr)
//    {
//        delete _ptex;
//        _ptex = nullptr;
//    }
    _ptex.reset();

	_tileCount = _tileW = _tileH = _tileWOffset = _tileHOffset = 0;
	_tileDir = TILE_HORIZ;

	_init = false;
}

Uint32 Image::width() const
{
    return _ptex ? _ptex->width() : 0;
//    Uint32 format;
//    int access, w, h;
//    SDL_QueryTexture(_ptex, &format, &access, &w, &h);
//    return w;
}

Uint32 Image::height() const
{
    return _ptex ? _ptex->height() : 0;
//    Uint32 format;
//    int access, w, h;
//    SDL_QueryTexture(_ptex, &format, &access, &w, &h);
//    return h;
}

bool Image::create(unsigned int w, unsigned int h, Uint32 nTiles /*=1*/, SDL_Color cAlphaKey /*=ALPHA_COLOUR*/, Uint8 iAlpha /* = 255 */ )
{
    bool bOk = false;
    Surface surface;
    if (surface.create(w, h))
    {
        bOk = initImage(&surface, nTiles, cAlphaKey, iAlpha);
    }
    return bOk;
}

//create this image from another image or part image (tile)
void Image::cloneFrom(Image &image, int iAlpha /*=-1*/)
{
    Rect r( 0, 0, image.width(), image.height() );
    cloneFrom(image, r, iAlpha);
}

void Image::cloneFrom(Image &image, Rect &r, int iAlpha /*=-1*/)
{
	cleanUp();

//	_init = Surface::create(r.width(), r.height(), iAlpha);
//
//	if (image.surface()->format->Amask && iAlpha!=-1)
//		//source image has alpha so set alpha in this new dest image too
//		SDL_SetAlpha(this->_surface, SDL_SRCALPHA, iAlpha);
//	else
//		//prefill with alpha colour so the final surface contains it where curr see through
//		ppg::drawSolidRect(this->_surface, 0, 0, r.width(), r.height(), ALPHA_COLOUR);
//
//    setTileCount(image.tileCount(), image.tileDir());
//
//	SDL_Rect sdlR = r.toSDL(); //{r.left(), r.top(), r.width(), r.height()};
//	ppg::blit_surface(image._surface, &sdlR, this->_surface, 0, 0);   //into "this" newly created 'copy'

}

/*
//create this image from another image (tile)
bool Image::cloneFrom(Image &image, int tileNum , int iAlpha )
{
	if (image.surface()->format->Amask || iAlpha != -1)
	{
		cleanUp();

		SDL_Surface *s;
//		s = SDL_CreateRGBSurface(SCREEN_SURFACE|(iAlpha>=0)?SDL_SRCALPHA:SDL_SRCCOLORKEY, image.tileW(), image.tileH(), SCREEN_BPP,
		s = SDL_CreateRGBSurface(SCREEN_SURFACE, image.tileW(), image.tileH(), SCREEN_BPP,
			0, 0, 0, 0);
#if 0
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif
#endif
		int alpha = image.surface()->format->alpha;

		SDL_SetAlpha(s, 0, 0);
		_surface = SDL_DisplayFormat( s );
//		_surface = s;
//		SDL_BlitSurface(image.surface(), NULL, _surface, NULL);
//		blitFrom(&image, tileNum);	//into this newly created 'copy'

		SDL_BlitSurface( image.surface(), (tileNum<0)?NULL:&image.tile(tileNum), _surface, 0 );
//		blit_surface(image.surface(), (tileNum<0)?NULL:&source->tile(tileNum),	//source
//				this->_surface, destX, destY);									//dest

		SDL_SetAlpha(image.surface(), SDL_SRCALPHA, alpha);
		return true;
	}

	cleanUp();
	_init = Surface::create(image.tileW(), image.tileH(), iAlpha);
	if (!_init) return false;

	if (!image.surface()->format->Amask || iAlpha == -1)
		//prefill with alpha colour so the final surface contains it where curr see through
		drawSolidRect(0,0,image.tileW(),image.tileH(), ALPHA_COLOUR);
	if (-1 == tileNum)
		setTileSize();
	else
		setTileSize(image.tileW(), image.tileH());

	if (!image.surface()->format->Amask || iAlpha == -1)
		blitFrom(&image, tileNum);	//into this newly created 'copy'

	return true;
}

//copy image class (also called from operator=)
bool Image::copy(Image &image)
{
	if (this == &image) return false;
	Surface::copy((Surface&)image);	//copy pointer (inc ref count)
	_init = true;
	return setTileSize(image._tileW, image._tileH);
}


*/

//cAlphaKey is colour to be used as transparent pixel colour, default putrid purple
//iAlpha is level of transparency (for all pixels) 0=full, 255=opaque
bool Image::load(const std::string &fileName, Uint32 nTiles /*=1*/, SDL_Color cAlphaKey /*=ALPHA_COLOUR*/, Uint8 iAlpha /* = 255 */ )
//bool Image::load(const std::string &fileName, int iAlpha /* =-1 */, Uint32 nTiles /*=1*/)
{
	//Temporary storage for the image that's loaded
	Surface newSurface;
	if (newSurface.load(fileName))
    {
        return initImage(&newSurface, nTiles, cAlphaKey, iAlpha);
    }
	return false;
}

//use the surface classes initSurface() fn to set up the image surface
bool Image::initImage(Surface *newSurface, Uint32 nTiles, SDL_Color cAlphaKey, Uint8 iAlpha)
{
	cleanUp();

    newSurface->setAlphaTransparency(iAlpha);
    newSurface->setTransparentColour(cAlphaKey);

	if (createTexFromSurface(newSurface))
    {
        _init = true;

        setTileCount(nTiles);   //default tile dir (until changed)
    }
    return _init;
}

bool Image::createTexFromSurface(Surface *s)
{
//    if (_ptex)
//    {
//        delete _ptex;
//        _ptex = nullptr;
//    }
//    _ptex = new Texture(*s);

    _ptex = std::shared_ptr<Texture>(new Texture(*s));
    return (_ptex != nullptr);
}

////assumes a single row of tiles [0,1,2,3,4,5...]
////static version, returns rect of exactly what is passed in
//SDL_Rect Image::tileRect(int i, int w, int h)
//{
//	SDL_Rect rect;
//	rect.x = i*w;
//	rect.y = 0;
//	rect.w = w;
//	rect.h = h;
//	return rect;
//}

//non static version that relies on setTileSize() previously called
SDL_Rect Image::tileRect(Uint32 tile)
{
	if (tile >= _tileCount) tile = 0;
	SDL_Rect rect;
	rect.x = tile*_tileWOffset;    //w * tileW, or 0
	rect.y = tile*_tileHOffset;    //h * tileH, or 0
	rect.w = _tileW;
	rect.h = _tileH;
	return rect;
}

//try to calc tile sizes depending on number of tiles and the direction of the repeating tiles
void Image::setTileCount(Uint32 nTiles, eTileDir tileDirection /*= TILE_HORIZ*/)
{
    if (tileDirection == TILE_HORIZ)
        setTileSize( (Uint32)(width() / ((nTiles>0)?nTiles:1)), 0, tileDirection );	//w=pixels/frames, h=default full height
    else
        setTileSize( 0, (Uint32)(height() / ((nTiles>0)?nTiles:1)), tileDirection );	//h=pixels/frames, w=default full width
}

//set the predefined size of a tile
//Currently tiles in images should be placed horizontally [0,1,2,3,4,5...]
bool Image::setTileSize(Uint32 w /*= 0*/, Uint32 h /*= 0*/, eTileDir tileDirection /*= TILE_HORIZ*/)
{
	if (!initDone()) return false; //can't set dimentions of no image

	if ((h < 1) || (h > height())) h = height();	//default to height of image
	if ((w < 1) || (w > width())) w = width();	//default to whole width of image

    _tileDir = tileDirection;
	_tileW = w;
	_tileH = h;

    if (_tileDir == TILE_HORIZ)
    {
        _tileHOffset = 0;
        _tileWOffset = _tileW;
        _tileCount = (w==0) ? 0 : (Uint32)width() / w;  //for a sanity check in tileRect() fn
    }
    else
    {
        _tileHOffset = _tileH;
        _tileWOffset  = 0;
        _tileCount = (h==0) ? 0 : (Uint32)height() / h;  //for a sanity check in tileRect() fn
    }

	return true;
}

