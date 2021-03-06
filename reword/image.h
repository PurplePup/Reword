//image.h

#if !defined IMAGE_H
#define IMAGE_H

#include <SDL.h>
#include <SDL_image.h>	//for IMG_ functions
#include "surface.h"
#include "utils.h"

#include <string>
#include <memory>

const SDL_Color ALPHA_COLOUR	= {0xFF,0x00,0xFF,0};

class Image //: public Surface
{
public:
    enum eTileDir { TILE_HORIZ, TILE_VERT };

	Image();
//	Image(const Image &img);
	Image(unsigned int w, unsigned int h, Uint32 nTiles = 1, SDL_Color cAlphaKey = ALPHA_COLOUR, Uint8 iAlpha = 255);
	Image(const std::string &fileName, Uint32 nTiles = 1, SDL_Color cAlphaKey = ALPHA_COLOUR, Uint8 iAlpha = 255);
    Image(Surface &surface, Uint32 nTiles = 1, SDL_Color cAlphaKey = ALPHA_COLOUR, Uint8 iAlpha = 255);
	virtual ~Image();

//	Image& operator=(const Image& img)
//	{
//	    if (this != &img)      // not same object
//		{
//			*(this->_ptex) = *img.tex();
//		}
//		return *this;
//	};

//    Image(Image&& other)    //move ctor
//    {
//        if (this != &other)
//        {
//            *(this->_ptex) = std::move(other._ptex);
//        }
//        return *this;
//    }

    bool create(unsigned int w, unsigned int h, Uint32 nTiles = 1, SDL_Color cAlphaKey = ALPHA_COLOUR, Uint8 iAlpha = 255);
	bool initDone() const { return _init; }	//has Image been initialised properly
    Texture * texture() const { return _ptex.get(); }

    void cloneFrom(Image &image, int iAlpha = -1);
    void cloneFrom(Image &image, Rect &r, int iAlpha = -1);

//	virtual bool load(const std::string &fileName, int iAlpha = -1, Uint32 nTiles = 1);	//default no alpha
    bool load(const std::string &fileName, Uint32 nTiles = 1, SDL_Color cAlphaKey = ALPHA_COLOUR, Uint8 iAlpha = 255);

	Uint32 width() const;
	Uint32 height() const;

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
	bool initImage(Surface *newSurface, Uint32 nTiles = 1, SDL_Color cAlphaKey = ALPHA_COLOUR, Uint8 iAlpha = 255);
	bool setTileSize(Uint32 w = 0, Uint32 h = 0, eTileDir tileDirection = TILE_HORIZ);
    bool createTexFromSurface(Surface *s);

private:
	bool    _init;
	Uint32  _tileCount, _tileW, _tileH, _tileWOffset, _tileHOffset;
    eTileDir _tileDir;

    std::shared_ptr<Texture> _ptex;
};

typedef std::shared_ptr<Image> tSharedImage;

//typedef std::unique_ptr<Image> tUniqueImage;
typedef std::map<Uint32, tSharedImage> tImageMap;


#endif //IMAGE_H
