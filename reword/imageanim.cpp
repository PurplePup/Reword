////////////////////////////////////////////////////////////////////
/*

File:			imageanim.cpp

Class impl:		ImageAnim

Description:	A class to handle image animation and/or movement

				Has additional features to provide auto movement and animation
				allowing simple control of items sliding in or out of the screen
				with minimum fuss and programming.

				Surface->Image->ImageAnim

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4.0	18.08.2007	Finally implemented the animation functions & types

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
#include "imageanim.h"
#include "utils.h"
#include <math.h>

ImageAnim::ImageAnim() :
	Image(), _x(0.0), _y(0.0f),
	_frame(0), _firstFrame(0), _lastFrame(0), _nFrames(0), _frameDir(1),
	_repeat(1), _restart(0), _visible(true), _pauseA(true), _rateA(0), _restartA(0),
	_delayA(0), _bDelayRestart(false)
{
	//default _nFrames = 0 so no anim possible, until setMaxFrame called
}

ImageAnim::ImageAnim(std::string fileName, bool bAlpha, Uint32 nFrames) :
	Image(fileName, bAlpha), _x(0.0f), _y(0.0f),
	_frame(0), _firstFrame(0), _lastFrame(0), _nFrames(0), _frameDir(1),
	_repeat(1), _restart(0), _visible(true), _pauseA(true), _rateA(0), _restartA(0),
	_delayA(0), _bDelayRestart(false)
{
	//_nFrames curr set to 0 in case Image class not initialised,
	//properly (due to bad image file etc), so now if it did init properly,
	//set the max number of frames, as passed in to this ctor
	if (Image::initDone()) setMaxFrame(nFrames);
}

ImageAnim::ImageAnim(const Image &img) :
	Image(img), _x(0.0f), _y(0.0f),
	_frame(0), _firstFrame(0), _lastFrame(0), _nFrames(0), _frameDir(1),
	_repeat(1), _restart(0), _visible(true), _pauseA(true), _rateA(0), _restartA(0),
	_delayA(0), _bDelayRestart(false)
{
}

bool ImageAnim::load(std::string fileName, int iAlpha, Uint32 nFrames)	//default no alpha, 1 frames
{
	Image::load(fileName, iAlpha);
	if (!Image::initDone()) return false;
	setMaxFrame(nFrames);
	return true;
}

void ImageAnim::setMaxFrame(Uint32 nFrames)
{
	setTileSize( (int)(width() / ((nFrames>0)?nFrames:1)), 0 );	//w=pixels/frames, h=default all
	_nFrames = tileCount();
}

//helper fn making anim startup easier
//params:
//firstFrame - 0+
//lastFrame - 0+ or -1 for all frames
//animType - see ImageAnim::eAnim
//rate - ms per frame
//repeat_N		- repeat N times or 0 for forever (default 0)
//restart_ms	- restart the loop in N ms after starting (default 0 - disabled)
//delay ms      - time to wait before anim starts (caller should use setAnimDelay(ms, bool) to set repeat)
void ImageAnim::startAnim(int firstFrame_0, int lastFrame_N, eAnim animType,
                          Uint32 rate_ms, Uint32 repeat_N, Uint32 restartIn_ms, Uint32 delayStart_ms)
{
	setFrameRange((firstFrame_0<0)?0:firstFrame_0, (lastFrame_N<0)?_nFrames-1:lastFrame_N);
	setFrame(firstFrame_0);
	setAnimRate(rate_ms);
	setAnimType(animType);
	setAnimRestart(restartIn_ms);
	setAnimDelay(delayStart_ms);
	setRepeat(repeat_N);

	//finally unpause animation if animation is possible
	pauseAnim( !canAnim() );		//set pause = false if anim possible (ie > 1 frame)
}

//set the current working range of frames (within the possible 0..N) frames loaded
void ImageAnim::setFrameRange(Uint32 firstFrame, Uint32 lastFrame)
{
	_firstFrame = (firstFrame>=_nFrames)?0:firstFrame;	//make sure its not > num frames
	_lastFrame = (lastFrame>=_nFrames)?_nFrames-1:lastFrame;
}

void ImageAnim::setFrame(Uint32 frame)
{
	_frame = (frame < _nFrames)?frame:0;	//must be valid frame num
}

void ImageAnim::setAnimType(eAnim anim)
{
	//set up the work function pointer depending on the type

	switch (anim)
	{
		case ANI_ONCE:	//
			pWorkFn = &ImageAnim::workONCE;		//iterate once then pause
			break;
		case ANI_HIDE:
			pWorkFn = &ImageAnim::workHIDE;		//iterate once then hide
			break;
		case ANI_LOOP:
			pWorkFn = &ImageAnim::workLOOP;		//iterate & repeat
			break;
		case ANI_REVERSE:
			pWorkFn = &ImageAnim::workREVERSE;	//iterate then reverse iterate (repeat)
			break;

		default:
			pWorkFn = &ImageAnim::workNONE;		//static, not moving
			break;
	}
}

void ImageAnim::work()
{
	if (!_delayA.done(false)) return;

	if (_restart && _restartA.done(true))
	{
		//start again... reset frame etc
		setFrame(_firstFrame);
		pauseAnim(false);
		setVisible(true);	//caller must reset _restart to prevent any more anim
//       _delayA.done(_bDelayRestart);
	}

	if (!_waitA.done(true)) return;

	//update the frame (if more than 1 and not paused)
	if ( canAnim()  && !_pauseA )
		(*this.*pWorkFn)();
}

void ImageAnim::workNONE(void)
{
	//do nothing
}
void ImageAnim::workONCE(void)
{
	if (_frameDir > 0)
	{
		//adding to get next frame
		if (_frame >= _lastFrame)
		{
			pauseAnim();
			return;
		}
	}
	else
	{
		//subtracting to get prev frame
		if (_frame <= _firstFrame)
		{
			pauseAnim();
			return;
		}
	}
	_frame += _frameDir;

}
void ImageAnim::workHIDE(void)
{
	if (_frameDir > 0)
	{
		//adding to get next frame
		if (_frame >= _lastFrame)
		{
			pauseAnim();
			setVisible(false);
			return;
		}
	}
	else
	{
		//subtracting to get prev frame
		if (_frame <= _firstFrame)
		{
			pauseAnim();
			setVisible(false);
			return;
		}
	}
	_frame += _frameDir;
}
void ImageAnim::workLOOP(void)
{
	if (_frameDir > 0)
	{
		//adding to get next frame
		if (_frame >= _lastFrame)
		{
			if (_repeat && _repeat-- <= 1) pauseAnim();
			_frame = _firstFrame;
			return;
		}
	}
	else
	{
		//subtracting to get prev frame
		if (_frame <= _firstFrame)
		{
			if (_repeat && _repeat-- <= 1) pauseAnim();
			_frame = _lastFrame;
			return;
		}
	}
	_frame += _frameDir;
}
void ImageAnim::workREVERSE(void)
{
	if (_frameDir > 0)
	{
		//adding to get next frame, if reached last frame, reverse dir
		if (_frame >= _lastFrame)
		{
			if (_repeat && _repeat-- <= 1) { pauseAnim(); return; }
			_frameDir = -1;
		}
	}
	else
	{
		//subtracting to get prev frame, if reached first frame, reverse dir
		if (_frame <= _firstFrame)
		{
			if (_repeat && _repeat-- <= 1) { pauseAnim(); return; }
			_frameDir = 1;
		}

	}
	_frame += _frameDir;
}

void ImageAnim::draw(Surface *s)
{
	if (_visible)
		blitTo(s, (int)round(_x), (int)round(_y), _frame);	//blit current frame to s at x, y
}

