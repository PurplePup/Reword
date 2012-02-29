#ifndef RESOURCE_H
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

    void setAlpha(int iAlpha);

    bool precache(const std::string & imageFile, int iAlpha =-1,
                    Uint32 nTiles =1, Image::eTileDir tileDir = Image::TILE_HORIZ);
    tSharedImage &get(const std::string & imageFile);

protected:
    int             _alpha;
    tResourceMap    _cache;

};


//RESOURCES (image, ttf, sound...)
class Resource
{
public:
    static void     initImage();
    static ResourceImg& image();
    static tSharedImage& image(const std::string &imgFile);
    static void     registerImage(ResourceImg* res);
};

#endif // RESOURCE_H
