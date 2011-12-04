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


void PlayMainMenu::init(Input *input)
{
	setTitle("REWORD");
	setHelp("Press B to select option", GREY_COLOUR);
	addItem(MenuItem(0, GREEN_COLOUR, "Play", "Go for it"));
	std::string s = "Difficulty ";
	s += (_gd._diffLevel==DIF_EASY)?"(easy)":(_gd._diffLevel==DIF_MED)?"(med)":"(hard)";
	addItem(MenuItem(1, ORANGE_COLOUR, s, "How brave are you feeling?"));
	addItem(MenuItem(2, BLUE_COLOUR, "Highscores", "Past Heroes"));
	addItem(MenuItem(3, PURPLE_COLOUR, "Instructions", "How to play"));
	addItem(MenuItem(4, RED_COLOUR, "Exit", "Quit game"));
	setItem(_gd._mainmenuoption);
	PlayMenu::init(input);
}

void PlayMainMenu::choose(MenuItem i)
{
	_gd._mainmenuoption = i._id;
	switch (i._id) {
		case 0: _gd._state = ST_MODE; break;
		case 1: _gd._state = ST_DIFF; break;
		case 2: _gd._state = ST_HIGH; break;
		case 3: _gd._state = ST_INST; break;
		case 4: _gd._state = ST_EXIT; break;
	}
	_running = false;
}

