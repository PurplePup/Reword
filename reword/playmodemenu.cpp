////////////////////////////////////////////////////////////////////
/*

File:			PlayModeMenu.cpp

Class impl:  	PlayModeMenu

Description:	A class derived from the PlayMenu interface to handle game mode
				selection

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			25 Feb 2008

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4.0	25.02.2008	implemented
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
#include "playmodemenu.h"

void PlayModeMenu::init(Input *input)
{
	setTitle("MODE");
	setHelp("Press B to select option", GREY_COLOUR);

	//it doesnt really matter what the id (first param) number is, but if we keep
	//0,1,2 as actual games and 99 for the optional resume, 255 for exit
	QuickState qs;
	bool bQSExists = qs.quickStateExists();
    if (bQSExists)
        addItem(MenuItem(99, GREEN_COLOUR, "Resume game...", "Continue playing a quick saved game"));

	addItem(MenuItem(0, PURPLE_COLOUR, "Reword", "Get shorter and all-letter words"));
	addItem(MenuItem(1, GOLD_COLOUR, "SpeedWord", "Get the all-letter words quickly to continue"));
	addItem(MenuItem(2, BLUE_COLOUR, "TimeTrial", "Get the most all-letter words in the time limit"));

	addItem(MenuItem(255, RED_COLOUR, "Back", "Back to main menu"));

    if (bQSExists)
        setItem(99);    //start at resume item if there
    else
       	setItem((int)_gd._mode);

	PlayMenu::init(input);
}


void PlayModeMenu::choose(MenuItem i)
{
	if (i._id < 3) {			// 0..2, so play
		_gd._mode = (eGameMode)i._id;
		_gd._state = ST_GAME;
	}
	else if (i._id == 99)		// optional (ie. found file) resume available
		_gd._state = ST_RESUME;
	else if (i._id == 255)		// exit & leave mode as-is
		_gd._state = ST_MENU;

	_running = false;	//exit this class running state
}

