//playdiff.h

#ifndef _PLAYDIFF_H
#define _PLAYDIFF_H

#include "i_play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "states.h"
#include "roundels.h"
#include "playmenu.h"
#include "controls.h"

class PlayDiff : public PlayMenu
{
public:
	PlayDiff(GameData& gd) : PlayMenu(gd) {}

    virtual void init(Input *input);
    virtual void choose(MenuItem i);
    virtual void render(Screen *s);
    virtual void work(Input* input, float speedFactor);

	virtual bool touch(const Point &pt);
    virtual bool tap(const Point &pt);

protected:

	Controls    _controlsDiff;

};

#endif //_PLAYDIFF_H
