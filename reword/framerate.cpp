////////////////////////////////////////////////////////////////////
/*

File:			framerate.cpp

Class impl:		Framerate

Description:	A class to determine speed/movement factors per frame

Author:			Al McLuckie (al-at-purplepup-dot-org)
				Derived from gamedev.net article by Ben Dilts: benbeandogdilts@cs.com

Date:			16 April 2007

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
#include "framerate.h"

Framerate::Framerate() :
	fps_last(SDL_GetTicks()), fps_count(0), fps_current(0)
{
}


void Framerate::init(float targetFps)
{
	_targetFps = targetFps;
	_frameDelay = SDL_GetTicks();
	_ticksPerSecond = SDL_GetTicks();
}

void Framerate::setSpeedFactor()
{
	_currentTicks = SDL_GetTicks();
	//This frame's length out of desired length
	_speedFactor = (float)(_currentTicks-_frameDelay)/((float)_ticksPerSecond/_targetFps);
	_fps = _targetFps/_speedFactor;
	if (_speedFactor <= 0)
	_speedFactor = 1;

	_frameDelay = _currentTicks;


	//add a frame, calc the next frame rate
	++fps_count;
//	if (SDL_GetTicks() - fps_last > 1000)
	if (_currentTicks - fps_last > 1000)
	{
		//at least 1 second has elapsed
		fps_last = SDL_GetTicks();
		fps_current = fps_count;
		fps_count = 0;
	}

}

void Framerate::capFrames()
{
	//simple cap frame rate @ NNfps - ##TODO## - use TimeBasedAnimation see elsewhere
	_ticks = SDL_GetTicks();
	if (_ticks - _currentTicks < (Uint32)(1000/_targetFps))
		SDL_Delay((Uint32)( (1000/_targetFps) - (_ticks - _currentTicks) ));	
}


