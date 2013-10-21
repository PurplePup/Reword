#if !defined RESOURCE_H
#define RESOURCE_H

#include "image.h"	//defines tSharedImage
#include <map>

class ResourceImg
{
    typedef std::map<std::string, tSharedImage> tResourceMap;
public:

    ResourceImg();
    ~ResourceImg();
    void clear();

    //void setAlpha(SDL_Color cAlphaKey = ALPHA_COLOUR, int iAlpha = 255);

    bool add(const std::string & imageFile, Uint32 nTiles = 1,
                        SDL_Color cAlphaKey = ALPHA_COLOUR, Uint8 iAlpha = 255,
                        Image::eTileDir tileDir = Image::TILE_HORIZ);
    tSharedImage &get(const std::string & imageFile);

protected:
    Uint32          _tiles;
    Uint8           _alpha;
    SDL_Color       _alphaKey;
    Image::eTileDir _tileDir;

    tResourceMap    _cache;
};


//RESOURCES (image, ttf, sound...)
class Resource
{
public:
//  static void             initImage();
    static ResourceImg&     image();
    static tSharedImage&    image(const std::string &imgFile);
    static void             registerImage(ResourceImg* res);
};

#endif // RESOURCE_H
