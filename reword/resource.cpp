////////////////////////////////////////////////////////////////////
/*

File:			resource.cpp

Class impl:		Resource

Description:	A image/resource cache to hold and provide images for sprites and
                static image objects etc

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			16 Feb 2012

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

#include "resource.h"
#include "platform.h"
#include <iostream>

ResourceImg::ResourceImg() : _alpha(-1)
{
    //ctor
}

ResourceImg::~ResourceImg()
{
    clear();
}

void ResourceImg::clear()
{
    //tResourceMap::iterator it = _cache.begin();
    //tResourceMap::iterator end = _cache.end();
    //for ( ; it != end; ++it)
    //{
    //    delete it->second;
    //}
    _cache.clear();
}

//set the default alpha to be used if get needs to
void ResourceImg::setAlpha(int iAlpha)
{
    _alpha = iAlpha;
}

//precache/load an image with a specific alpha value. Returns false if image not found
//DO NOT use filepath, just file name as path gets added in get()
bool ResourceImg::add(const std::string & imageFile, int iAlpha /*=-1*/,
                           Uint32 nTiles /* =1 */, Image::eTileDir tileDir /*= TILE_HORIZ*/)
{
    int oldAlpha = _alpha;
    _alpha = iAlpha;
    tSharedImage img = get(imageFile);
    _alpha = oldAlpha;

    if (!img->initDone())
    {
        std::cout << "Resource precache image " << imageFile << " failed!" << std::endl;
        return false;  //false if image not loaded
    }

    img->setTileCount(nTiles, tileDir);
    return true;
}

//return a pointer to a located image resource. If not precached
//(ie not found in map), will load the image before inserting it
//into the map.
tSharedImage &ResourceImg::get(const std::string & imageFile)
{
    tResourceMap::iterator it = _cache.find(imageFile);
    if (it == _cache.end())
    {
        //not found so try to load
        tSharedImage image = tSharedImage(new Image(RES_IMAGES + imageFile, _alpha));
        it = _cache.insert(it, std::make_pair(imageFile, image));

#if _DEBUG
std::cout << "# of images " << _cache.size() << " - added " << image->_dbgName << std::endl;
#endif
    }
    return it->second;
}



//////////////////////////////// Resource Locator/////////////////////////////

static ResourceImg * _img = 0;
static ResourceImg _nullimg;

void Resource::initImage()
{
    _img = &_nullimg;
}
ResourceImg& Resource::image()
{
    assert(_img != NULL);
    return *_img;
}
tSharedImage& Resource::image(const std::string &imgFile)
{
    assert(_img != NULL);
    return _img->get(imgFile);
}
void Resource::registerImage(ResourceImg* img)
{
    if (img == NULL)
        _img = &_nullimg;   // revert to null service
    else
        _img = img;
}


