////////////////////////////////////////////////////////////////////
/*

File:			spritemgr.cpp

Class impl:		SpriteMgr

Description:	A container class for a list of sprites, to be updated regularly during
				the work() / draw() cycle



				##### WORK IN PROGRESS - CURRENTLY NOT (YET) USED IN REWORD #####



Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			18 April 2007

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

#include "spritemgr.h"

#include "screen.h"
#include "global.h"
#include "resource.h"

//#include <iostream> //for debug msgs
#include <cassert>

SpriteMgr::SpriteMgr() :
	_handle(0)
{
}

SpriteMgr::~SpriteMgr()
{
	clear();
}

void SpriteMgr::clear()
{
	//delete all sprites previously added to the manager
	for (_it = _sprmap.begin(); _it != _sprmap.end(); ++_it)
	{
	    del( (*_it).first );  //handle
	}

	_handle = 0;
}

//get a sprite pointer using the handle given
Sprite * SpriteMgr::get(Uint32 handle)
{
	_it = _sprmap.find(handle);
	if (_it == _sprmap.end()) return nullptr;
	return (*_it).second._spr.get();
}

//add a sprite and return the (unique) allocated handle
Uint32 SpriteMgr::add(t_pSharedSpr &pSpr)
{
	if (!pSpr.get()) return 0;

    SpriteX sx;
    sx._spr = pSpr;
	_sprmap[++_handle] = sx;

    pSpr->setObjectId(_handle);  //used to lookup spr after event return
    pSpr->_sigEvent2.Connect(this, &SpriteMgr::SpriteEvent);

	return _handle;
}

//add using image resource name and initial params to start anim etc
Sprite * SpriteMgr::add(const std::string resource, int x, int y, Uint32 rate_ms, ImageAnim::eAnim anim)
{
    if (resource.empty())
    {
        assert(false);  //fail!
        return 0;
    }
    t_pSharedSpr pSpr(new Sprite(Resource::image(resource)));

    SpriteX sx;
    sx._spr = pSpr;
	_sprmap[++_handle] = sx;

    pSpr->setObjectId(_handle);  //used to lookup spr after event return
    pSpr->_sigEvent2.Connect(this, &SpriteMgr::SpriteEvent);

    pSpr->startAnim( 0, -1, anim, rate_ms);
    pSpr->setPos(x, y);

	return pSpr.get();
}

bool SpriteMgr::del(Uint32 handle)
{
	t_SprMap::iterator it = _sprmap.find(handle);
	return del(it);
}

bool SpriteMgr::del(t_SprMap::iterator it)
{
	if (it == _sprmap.end()) return false;

    if ((*it).second._spr.get())
    {
        (*it).second._spr->_sigEvent2.Disconnect(this, &SpriteMgr::SpriteEvent);
        (*it).second._spr.reset(); //delete
    }
	_sprmap.erase(it);
	return true;
}


void SpriteMgr::work(Uint32 handle)
{
   	t_SprMap::iterator it = _sprmap.find(handle);
	if (it == _sprmap.end()) return;

    if ((*it).second._bDead)
    {
        del(it);
        return;
    }

	//find actual sprite and update it
	Sprite *s = (*it).second._spr.get();
	if (s)
	{
        s->work();
	}
}

void SpriteMgr::work(void)
{
	//update all in map
	_itend = _sprmap.end();
	for (_it = _sprmap.begin(); _it != _itend; ++_it)
	{
        if ((*_it).second._bDead)
        {
            del(_it);
            return;
        }

        (*_it).second._spr->work();
	}
}

void SpriteMgr::draw(Uint32 handle, Screen* screen)
{
	//find actual sprite and draw it
	Sprite *s = get(handle);
	if (s) s->draw(screen);
}

void SpriteMgr::draw(Screen* screen)
{
	//draw all in map
	_itend = _sprmap.end();
	for (_it = _sprmap.begin(); _it != _itend; ++_it)
	{
        if ((*_it).second._spr.get())
            (*_it).second._spr->draw(screen);
	}
}

void SpriteMgr::SpriteEvent(int event, int spr_id)
{
    if (event == USER_EV_END_DELETE)
    {
        //remove the sprite from the map
        t_SprMap::iterator it = _sprmap.find(spr_id);
        if (it == _sprmap.end()) return;

        (*it).second._bDead = true;    //delete next work cycle
    }
}





