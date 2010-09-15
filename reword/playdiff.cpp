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
#include "states.h"
#include "utils.h"
#include <sstream>

void PlayDiff::init(Input *input)
{
	setTitle("LEVEL");
	setHelp("Press B to select difficulty", GREY_COLOUR);
	addItem(MenuItem(DIF_EASY, GREEN_COLOUR,
		"Easy", ""));
	addItem(MenuItem(DIF_MED, ORANGE_COLOUR,
		"Medium", ""));
	addItem(MenuItem(DIF_HARD, RED_COLOUR,
		"Hard", ""));
	setItem((int)_gd._diffLevel);
	PlayMenu::init(input);
}

void PlayDiff::choose(MenuItem i)
{
	_gd.setDiffLevel((eGameDiff)i._id);
	_gd._state = ST_MENU;
	_running = false;	//exit this class running state
}

void PlayDiff::render(Screen *s)
{
	PlayMenu::render(s);
	MenuItem i = getSelected();
	std::stringstream times;
	times << "Reword " << ((int)DIF_MAX - i._id)*COUNTDOWN_REWORD << " sec, " <<
			  "Speed6 " << ((int)DIF_MAX - i._id)*COUNTDOWN_SPEED6 << ", " <<
			  "TimeTrial " << ((int)DIF_MAX - i._id)*COUNTDOWN_TIMETRIAL;

	int yy(getNextYPos());
	int htPlusGap = _gd._fntClean.height() + ( _gd._fntClean.height()/6 );
	switch (i._id)
	{
	case DIF_EASY:
		_gd._fntClean.put_text(s, yy, times.str().c_str(), GREEN_COLOUR, true);
		_gd._fntClean.put_text(s, yy+htPlusGap, "Only easy words used. Can pause game.", GREEN_COLOUR, true);
		break;
	case DIF_MED:
		_gd._fntClean.put_text(s, yy, times.str().c_str(), ORANGE_COLOUR, true);
		_gd._fntClean.put_text(s, yy+htPlusGap, "Also includes easy words. No pause.", ORANGE_COLOUR, true);
		break;
	case DIF_HARD:
		_gd._fntClean.put_text(s, yy, times.str().c_str(), RED_COLOUR, true);
		_gd._fntClean.put_text(s, yy+htPlusGap, "Also easy and medium words. No Pause", RED_COLOUR, true);
		break;
	default:break;
	}
}