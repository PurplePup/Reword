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
	Sprite *_s;

    //some iterators for use in updates
	tSprMap::iterator _it;	    //not thread safe (but we're not using threads so...)
	tSprMap::iterator _itend;	//not thread safe
};

//class to add a sprite animation, play it then delete it
//to allow one off effect animations to be run
class OneShot
{
public:
    typedef std::vector<Sprite *> tSpriteList;

    OneShot();
    ~OneShot();

    void        add(Sprite* spr);
	void		work(void);
	void		draw(Screen* s);
    void        event(int, int);

protected:

    tSpriteList         _list;
    std::vector<int>    _unused;
};



#endif //_SPRITEMGR_H

