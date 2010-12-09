//image.h

#ifndef IMAGE_H
#define IMAGE_H

#include "SDL.h"
#include "SDL_image.h"	//for IMG_ functions
#include <string>
#include "surface.h"

const SDL_Color ALPHA_COLOUR	= {0xFF,0x00,0xFF,0};

class Image : public Surface
{
public:
    enum eTileDir { TILE_HORIZ, TILE_VERT };

	Image();
	Image(unsigned int w, unsigned int h, int iAlpha = -1);
	Image(std::string fileName, int iAlpha = -1);
	Image(const Image &img);
	virtual ~Image();

	Image& operator=(const Image& img)
	{
	    if (this != &img)      // not same object
		{
			*(this->_surface) = *img.surface();
			this->_tileW = img._tileW;
			this->_tileH = img._tileH;
			this->_tileCount = img._tileCount;
		}
		return *this;
	};

	bool initDone() { return _init; }	//has Image been initialised properly
	bool initImage(SDL_Surface *newSurface, int iAlpha = -1);

	Image * createImageFromThis(int tileNum, int iAlpha = -1);	//return a new image from a tile in this image
	void createThisFromImage(Image &image, int tileNum = -1, int iAlpha = -1);	//create this image using an existing image (tile)

	virtual bool load(std::string fileName, int iAlpha = -1);	//default no alpha
	bool setTileSize(int w = 0, int h = 0, eTileDir tileDir = TILE_HORIZ);
	SDL_Rect tile(int i);	//uses stored _tileW and _tileH

	//specific blit for Image class types
	void blitTo(Surface* dest, int destX = 0, int destY = 0, int tileNum = -1);
	void blitFrom(Image* source, int tileNum = -1, int destX = 0, int destY = 0);

//	SDL_Rect *clip() {return _clip;}
	int tileW() {return _tileW;}
	int tileH() {return _tileH;}
	int tileCount()	{return _tileCount;}

	//static functions available to all
	static SDL_Rect tile(int i, int w, int h);	//[static] just return the Rect for requested tile

protected:
	void cleanUp();

	int _tileW, _tileH; //actual individual tile w & h withing image
	int _tileCount;	    //number of tiles in image (if setTileSize() used)

    int _tileWOffset, _tileHOffset;     //value or 0, depend on eTileDir to multiply by

private:
	bool _init;
};


#endif //IMAGE_H
