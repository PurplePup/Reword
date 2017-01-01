//PlayMainMenu.h

#if !defined _PlayMainMenu_H
#define _PlayMainMenu_H

#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "roundels.h"
#include "playmenu.h"
#include "fontttf.h"

class PlayMainMenu : public PlayMenu
{
public:

	PlayMainMenu(GameData& gd) : PlayMenu(gd) {}
    virtual void init(Input *input, Screen * scr);
    virtual void choose(MenuItem i);
    virtual void chooseDone();
    virtual void render(Screen *s);
};

#endif //_PlayMainMenu_H
