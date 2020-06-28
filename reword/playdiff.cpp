////////////////////////////////////////////////////////////////////
/*

File:			playdiff.cpp

Class impl:		PlayDiff

Description:	A class derived from the PlayMenu interface to handle all screen
				events and drawing of the Difficulty setting screen

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3.1	07.06.2007	Speed up menu movement a little
				0.4		17.03.2008	Changed description to use score consts
				0.5		16.05.2008	Implement PlayMenu base class and touch screen

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


#include "global.h"
#include "playdiff.h"
#include "resource.h"
#include "states.h"
#include "utils.h"
#include <sstream>

void PlayDiff::init(Input *input)
{
	setName("LEVEL");
	std::string msg = "Press " + input->keyDescription(ppkey::B) + " to select difficulty";
	setHelp(msg, GREY_COLOUR);
	addItem(MenuItem(DIF_EASY, GREEN_COLOUR,  "Easy", ""));
	addItem(MenuItem(DIF_MED,  ORANGE_COLOUR, "Medium", ""));
	addItem(MenuItem(DIF_HARD, RED_COLOUR,    "Hard", ""));
	setItem((int)_gd._diffLevel);
	//limit items to smaller area
	setMenuArea(Rect(0, BG_LINE_TOP, SCREEN_WIDTH, (int)(BG_LINE_TOP + ((BG_LINE_BOT - BG_LINE_TOP)*0.85))));

    //load EXIT/NEXT buttons
    {
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_exit_small.png")));
    p->setPos(8, BG_LINE_BOT + ((SCREEN_HEIGHT - BG_LINE_BOT - p->tileH())/2));
//    p->_sigEvent2.Connect(this, &PlayDiff::ControlEvent);
    Control c(p, CTRLID_EXIT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsDiff.add(c);
    }

	PlayMenu::init(input);
}

void PlayDiff::choose(MenuItem i)
{
    if (i.id() >= 0)
        _gd.setDiffLevel((eGameDiff)i.id());

	_gd._state = ST_MENU;
	_running = false;	//exit this class running state
}

void PlayDiff::render(Screen *s)
{
    //use base class to drawmenu items
	PlayMenu::render(s);

	//now render helpful messages underneath
	MenuItem i = getSelected();
	std::stringstream times;
	times << "Reword " << ((int)DIF_MAX - i.id())*COUNTDOWN_REWORD << " sec, " <<
			  "Speeder " << ((int)DIF_MAX - i.id())*COUNTDOWN_SPEEDER << ", " <<
			  "TimeTrial " << ((int)DIF_MAX - i.id())*COUNTDOWN_TIMETRIAL;

	int htAndGap = _gd._fntSmall.height() + (_gd._fntSmall.height()/4);
	int yy = BG_LINE_BOT - (htAndGap*2);     //(getNextYPos());
	switch (i.id())
	{
	case DIF_EASY:
		_gd._fntSmall.put_text(s, yy, times.str().c_str(), GREEN_COLOUR, true);     //10 sec pause penalty
		_gd._fntSmall.put_text(s, yy+htAndGap, "Only 'easy' words used. Can pause game.", GREEN_COLOUR, true);
		break;
	case DIF_MED:
		_gd._fntSmall.put_text(s, yy, times.str().c_str(), ORANGE_COLOUR, true);    //15 sec pause penalty
		_gd._fntSmall.put_text(s, yy+htAndGap, "Also includes 'easy' words. Can pause game.", ORANGE_COLOUR, true);
		break;
	case DIF_HARD:
		_gd._fntSmall.put_text(s, yy, times.str().c_str(), RED_COLOUR, true);
		_gd._fntSmall.put_text(s, yy+htAndGap, "Also 'easy' and 'medium' words. No Pause", RED_COLOUR, true);
		break;
	default:break;
	}

	_controlsDiff.render(s);
}

void PlayDiff::work(Input *input, float speedFactor)
{
    PlayMenu::work(input, speedFactor);

    _controlsDiff.work(input, speedFactor);
}

bool PlayDiff::touch(const Point &pt)
{
    //const int ctrl_id =
        _controlsDiff.touched(pt);    //needed to highlight a touched control

	return PlayMenu::touch(pt);
}

//releasing 'touch' press
bool PlayDiff::tap(const Point &pt)
{
    const int ctrl_id = _controlsDiff.tapped(pt);

    if (ctrl_id == CTRLID_EXIT)  //exit after anim faded
    {
        _gd._state = ST_MENU;	//back to menu
        _running = false;
        return true;
    }

    return PlayMenu::tap(pt);
}
