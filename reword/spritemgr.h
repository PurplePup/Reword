//spritemgr.h

#if !defined _SPRITEMGR_H
#define _SPRITEMGR_H

#include "sprite.h"

#include <map>

class Screen;

class SpriteMgr
{
public:
    struct SpriteX
    {
        SpriteX() : _bDead(false) {}
        bool            _bDead;
        t_pSharedSpr   _spr;
    };

	typedef std::map <Uint32, SpriteX> t_SprMap;

	SpriteMgr();
	~SpriteMgr();

	void clear();

	Sprite *    get(Uint32 handle);
	Uint32	 	add(t_pSharedSpr &pSpr);
	Sprite *    add(const std::string resource, int x=0, int y=0, Uint32 rate_ms=25, ImageAnim::eAnim anim=ImageAnim::ANI_ONCEDEL);
	bool		del(Uint32 handle);
    bool        del(t_SprMap::iterator it);

	void		work(Uint32 handle);
	void		work(void);				//for all sprites
	void		draw(Uint32 handle, Screen* screen);
	void		draw(Screen* screen);	//for all sprites

    void        SpriteEvent(int event, int spr_id);

private:
	t_SprMap	_sprmap;
	Uint32  	_handle;

    //some iterators for use in updates
	t_SprMap::iterator _it;	    //not thread safe (but we're not using threads so...)
	t_SprMap::iterator _itend;	//not thread safe
};

#endif //_SPRITEMGR_H

