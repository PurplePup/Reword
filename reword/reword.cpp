////////////////////////////////////////////////////////////////////
/*
Program:		REWORD
Author:			Al McLuckie (al-at-purplepup-dot-org)
Date:			06 April 2007


History:		Version	Date		Change
				-------	----------	--------------------------------
				0.1		06.09.2006	First version
				0.2		31.10.2006	Corrected scoring to always place higher scores
										first then #words within score, see isHiScore()
									Changed shade of orange for medium difficulty to stand
										out from red better.
									Used REWORDLIST to up the number of 6 letter words to
										~1700 by increasing all 3, 4, 5 and 6 letter words cutoff
										from 7 to 8 (which now *nearly* touches screen bottom)
									Reduce wait on BONUS/BADLUCK screen to 1.5 sec from 2
									Lighten letter roundel colour to see letters better.
										esp. for different viewing angles on gp2x.
									Dictionary mods to exclude more bad words and include
										missing ones.
									Fix resource leak - not releasing a surface
									Fix countdown bug in countdown_callback() when < 0 (in easy)
									Fixed launch script exit back to menu properly and 'sync'
										after exit from game
									Added loading splash so dont need to load ttf before showing
										a message. So reduce time showing black screen before
										'loading' message can be displayed.
									Improved random seed calculation.
									Change hiscore initials to difficulty level colour and allow
										other hiscore levles to be shown using joy left/right
				0.3		25.02.2007	Rewrote the game completely using C++ and a proper game framework
										allowing each screen to be derived from an interface and
										a controlled framerate to be introduced. Animated all movement
										aspects of the game to give a more 'arcade' feel to it.
										e.g. switching and selecting letters now move to their
											destination rather than jump.
									Revised rewordlist utility to just check for valid words etc and
										produce a simple list file that REWORD accesses directly, using
										CWords class. Also helps prevent people looking up the matching
										words during play once they know the 6 letter word.
									Various changes, see other sources and README.txt
				0.3.1	09.06.2007	Minor fix point release for v0.3
				0.4.0	17.03.2008	Added speeder and timetrial modes
				0.5.0	18.06.2008	Added touch screen support

Description:	A simple word game where you need to find as many words
				from a 6 letter word as possible in the time allowed.
				Only if you find (one of) the 6 letter words do you progress
				to the next level.

Extra Notes:	PAUSE causes a few second loss (pause mid game only in easy mode)
				100 point bonus if a 6 letter found
				If all words found, add (seconds remaining * a value)

##TODO##		----------------------------------------------------
				New sounds still needed
				poss new mode, to draw found/remaining words as wrapping round
					screen to allow more 6 letter words than the ~2800 currently
				better graphics needed - more transparency/anti-aliasing ?

				For next version:
				- Continue modding dictionary.
				- Add select dictionary option (for UK/US/Special etc, built with
					 rewordlist console application)


				locate and 'do' all ##TODO##
				locate and remove all ##DEBUG##

				-----------------------------------------------------

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

#include "stdio.h"
#include <string>
#include <iostream>
#include "SDL.h"		//include for SDLMain to be defined (as main)

#include "global.h"
#include "game.h"


int main(int argc, char* argv[]) // <- this must match exactly, since SDL rewrites it
{
	bool bHelp = false;
    GameOptions options;

	//v. simple loop to load cmd line args - in any order,
	//but must be seperately 'dashed' ie. -l -f not -lf
	for (int i = 1; i< argc; ++i)
	{
		std::string arg = argv[i];
		if ("--help" == arg)
		{
			bHelp = true;
			continue;
		}
		if ("-nosfx" == arg)
		{
			options._bSfx = false;
			continue;
		}
		if ("-nomusic" == arg)
		{
			options._bMusic = false;
			continue;
		}
		if ("-nosound" == arg)
		{
			options._bSfx = options._bMusic = false;
			continue;
		}
		//else ignore anything else for now
	}

    if (bHelp)
    {
		std::cout << "Reword game : " << VERSION_STRING << std::endl
				<< "Useage:" << std::endl
				<< "reword [params]" << std::endl
				<< std::endl
				<< "  Params:  " << std::endl
				<< "  -nosound  -  disable all sound (sfx and music)" << std::endl
				<< "  -nosfx    -  start with mute sound effects" << std::endl
				<< "  -nomusic  -  start with mute music" << std::endl
				<< std::endl;

        return 0;
    }


	Game game;
	if (game.init(options))
	{
		if (game.run()) return 0;
		//else fall through to return error
	}
	std::cerr << "Error : " << game.lastError() << std::endl;
	return 1;
}

