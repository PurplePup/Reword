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
		delete (*_it).second;
		_sprmap.erase(_it);
	}

	_handle = 0;
}

//get a sprite pointer using the handle given
Sprite * SpriteMgr::get(Uint32 handle)
{
	_it = _sprmap.find(handle);
	if (_it == _sprmap.end()) return NULL;
	return (*_it).second;
}

//add a sprite and return the (unique) allocated handle
Uint32 SpriteMgr::add(Sprite * spr)
{
	if (NULL == spr) return 0;
//	if (_sprmap.find(handle+1) == _sprmap.end()) ??? find next handle...
	_sprmap[++_handle] = spr;	//##TODO## will eventually wrap and reuse ....
	return _handle;
}

bool SpriteMgr::del(Uint32 handle)
{
	tSprMap::iterator it = _sprmap.find(handle);
	if (it == _sprmap.end()) return false;
	delete (*it).second;
	_sprmap.erase(it);
	return true;
}

void SpriteMgr::work(Uint32 handle)
{
	//find actual sprite and update it
	_s = get(handle);
	if (_s)
	{
//		if (_s->isLive()) _s->work(); else del(handle);
_s->work();
	}
}

void SpriteMgr::work(void)
{
	//update all in map
	for (_it = _sprmap.begin(); _it != _sprmap.end(); ++_it)
	{
//		if ((*_it).second->isLive()) (*_it).second->work(); else del((*_it).first);
(*_it).second->work();
	}
}

void SpriteMgr::draw(Uint32 handle, Screen* screen)
{
	//find actual sprite and draw it
	_s = get(handle);
	if (_s) _s->draw(screen);
}

void SpriteMgr::draw(Screen* screen)
{
	//draw all in map
	for (_it = _sprmap.begin(); _it != _sprmap.end(); ++_it, (*_it).second->draw(screen));
}


