//spritemgr.h

 
#ifndef _SPRITEMGR_H
#define _SPRITEMGR_H

#include "sprite.h"
#include "screen.h"

#include <map>

class SpriteMgr
{
public:
	typedef std::map <Uint32, Sprite*> tSprMap;

	SpriteMgr();
	~SpriteMgr();

	void clear();

	Sprite *	get(Uint32 handle);
	Uint32	 	add(Sprite* spr);
	bool		del(Uint32 handle);
	
	void		work(Uint32 handle);
	void		work(void);				//for all sprites
	void		draw(Uint32 handle, Screen* screen);
	void		draw(Screen* screen);	//for all sprites

private:
	tSprMap	_sprmap;
	Uint32	_handle;
	tSprMap::iterator _it;	//not thread safe (but we're not using threads so...)
	Sprite *_s;
};

#endif //_SPRITEMGR_H

