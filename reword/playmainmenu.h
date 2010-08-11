//PlayMainMenu.h
 
#ifndef _PlayMainMenu_H
#define _PlayMainMenu_H

#include "play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "roundels.h"
#include "playmenu.h"


class PlayMainMenu : public PlayMenu
{
public:

	PlayMainMenu(GameData& gd) : PlayMenu(gd) {}
    virtual void init(Input *input);
    virtual void choose(MenuItem i);

};

#endif //_PlayMainMenu_H
