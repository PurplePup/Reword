////////////////////////////////////////////////////////////////////
/*

File:			app.cpp

Class impl:		App

Description:	A class to initialise most of whats needeed for SDL games

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

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
#include <SDL.h>

#include "app.h"
#include "game.h"
  
App::App() :
	_init(false)
{
	atexit(SDL_Quit);	//auto cleanup, just in case 
}

App::~App()
{
}

bool App::init()
{
	if (_init) return true;

	//Init SDL but if anything borks, just exit
	if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 )
	{
		setLastError("Unable to init SDL");
		return false;
	}
	
	return (_init = true);
}

bool App::go(void)
{
	if (!init()) return false;	//in case init() not called

	Game game;
	if (game.init())
	{
		if (game.run()) return true;
		//else fall through to return error
	}
	setLastError(game.lastError());	//print very last err, if any
	return false;
}
