//PlayModeMenu.h

#if !defined _PlayModeMenu_H
#define _PlayModeMenu_H

#include "i_play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "playmenu.h"


class PlayModeMenu : public PlayMenu
{
public:

	PlayModeMenu(GameData& gd) : PlayMenu(gd) {}
    virtual void init(Input *input, Screen * scr);
    virtual void choose(MenuItem i);

};

#endif //_PlayModeMenu_H
