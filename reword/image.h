//image.h

#ifndef IMAGE_H
#define IMAGE_H

#include "SDL.h"
#include "SDL_image.h"	//for IMG_ functions
#include "surface.h"

#include <string>
#include <boost/shared_ptr.hpp>

const SDL_Color ALPHA_COLOUR	= {0xFF,0x00,0xFF,0};

class Image : public Surface
{
public:
    enum eTileDir { TILE_HORIZ, TILE_VERT };

	Image();
	Image(unsigned int w, unsigned int h, int iAlpha = -1);
	Image(const std::string &fileName, int iAlpha = -1, Uint32 nTiles = 1);
	Image(const Image &img);
	virtual ~Image();

	Image& operator=(const Image& img)
	{
	    if (this != &img)      // not same object
		{
			*(this->_surface) = *img.surface();
		}
		return *this;
	};

	bool initDone() const { return _init; }	//has Image been initialised properly
	bool initImage(SDL_Surface *newSurface, int iAlpha = -1, Uint32 nTiles = 1);

    void cloneFrom(Image &image, int iAlpha = -1);
    void cloneFrom(Image &image, Rect &r, int iAlpha = -1);

	virtual bool load(const std::string &fileName, int iAlpha = -1, Uint32 nTiles = 1);	//default no alpha

//	SDL_Rect *clip() {return _clip;}

	SDL_Rect tileRect(Uint32 tile);	//uses stored _tileW and _tileH
	eTileDir tileDir() const    { return _tileDir; }
	Uint32 tileW() const        { return _tileW; }
	Uint32 tileH() const        { return _tileH; }
	Uint32 tileCount()	const   { return _tileCount; }
    void setTileCount(Uint32 nTiles, eTileDir tileDirection = TILE_HORIZ);

#if _DEBUG
    std::string _dbgName;
#endif

protected:
	void cleanUp();
	bool setTileSize(Uint32 w = 0, Uint32 h = 0, eTileDir tileDirection = TILE_HORIZ);

private:
	bool    _init;
	Uint32  _tileCount, _tileW, _tileH, _tileWOffset, _tileHOffset;
    eTileDir _tileDir;
};

typedef boost::shared_ptr<Image> tSharedImage;

#endif //IMAGE_H
