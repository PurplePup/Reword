////////////////////////////////////////////////////////////////////
/*

File:			PlayMainMenu.cpp

Class impl:		PlayMainMenu

Description:	A class derived from the PlayMenu interface to handle all screen
				events and drawing of the main menu screen

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4.0	11.02.2008	implemented
				0.5		16.05.2008	Implement PlayMenu base class and touch screen
				0.7		02.01.17	Moved to SDL2

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
#include "playmainmenu.h"

#include <stdio.h>
#include <string>


void PlayMainMenu::init(Input *input, Screen * scr)
{
	setName("REWORD");

	addItem(MenuItem(0, GREEN_COLOUR, "Play", "Go for it", true));
	std::string s = "Difficulty ";
	s += (_gd._diffLevel==DIF_EASY)?"(easy)":(_gd._diffLevel==DIF_MED)?"(med)":"(hard)";
	addItem(MenuItem(1, ORANGE_COLOUR, s, "How brave are you feeling?", true));
	addItem(MenuItem(2, BLUE_COLOUR, "Highscores", "Past Heroes", true));
	addItem(MenuItem(3, PURPLE_COLOUR, "Instructions", "How to play", true));
	addItem(MenuItem(4, GOLD_COLOUR, "Options", "Settings and preferences", true));
	addItem(MenuItem(5, RED_COLOUR, "Exit !", "Quit game", true));
	setItem(_gd._mainmenuoption);

	std::string msg = "Press " + input->keyDescription(ppkey::B) + " to select item";
	setHelp(msg, GREY_COLOUR);

	PlayMenu::init(input, scr);
}

void PlayMainMenu::choose(MenuItem i)
{
	_gd._mainmenuoption = i.id();
	switch (i.id()) {
		case 0: _gd._state = ST_MODE; break;
		case 1: _gd._state = ST_DIFF; break;
		case 2: _gd._state = ST_HIGH; break;
		case 3: _gd._state = ST_INST; break;
		case 4: _gd._state = ST_OPTN; break;
		case 5:
//		case -1:    //kbd exit - commented out otherwise too easy to exit game
        default:    //should never happen
		{
		    exitMenu(); //start exit anim (all slide off)
		    return;     //don't set running false just yet
		}
	}
	_running = false;
}

void PlayMainMenu::chooseDone()
{
    _gd._state = ST_EXIT;   //and actually exit
	_running = false;
}

void PlayMainMenu::render(Screen *s)
{
    PlayMenu::render(s);

    //add version no - specific to main menu screen
    _gd._fntClean.put_text_right(s, 0, 5, VERSION_STRING, BLACK_COLOUR);
}
