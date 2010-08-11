//playdiff.h

#ifndef _PLAYDIFF_H
#define _PLAYDIFF_H

#include "play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "states.h"
#include "roundels.h"
#include "playmenu.h"

class PlayDiff : public PlayMenu
{
public:
	PlayDiff(GameData& gd) : PlayMenu(gd) {}

    virtual void init(Input *input);
    virtual void choose(MenuItem i);
    virtual void render(Screen *s);
};

#endif //_PLAYDIFF_H
