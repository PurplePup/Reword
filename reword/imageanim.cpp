////////////////////////////////////////////////////////////////////
/*

File:			imageanim.cpp

Class impl:		ImageAnim

Description:	A class to handle image animation and/or movement

				Has additional features to provide auto movement and animation
				allowing simple control of items sliding in or out of the screen
				with minimum fuss and programming.

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
#include "screen.h"
#include "imageanim.h"
#include "utils.h"

#include <cmath>
#include <cassert>
#include <algorithm>

ImageAnim::ImageAnim() :
	//_image(),
	_x(0.0), _y(0.0f), _objectId(0),
	_frame(0), _firstFrame(0), _lastFrame(0), _nFrames(0), _frameDir(1),
	_repeat(1), _restart(0), _inflateBy(0),
	_visible(true), _pauseA(true), _rateA(0), _restartA(0),
	_delayA(0), _bDelayRestart(false), _frameCustom(0)
{
	//default _nFrames = 0 so no anim possible, until setFrameCount called
}

ImageAnim::ImageAnim(std::string fileName, bool bAlpha, Uint32 nFrames) :
	//_image(fileName, bAlpha),
	_x(0.0f), _y(0.0f), _objectId(0),
	_frame(0), _firstFrame(0), _lastFrame(0), _nFrames(0), _frameDir(1),
	_repeat(1), _restart(0), _inflateBy(0),
	_visible(true), _pauseA(true), _rateA(0), _restartA(0),
	_delayA(0), _bDelayRestart(false), _frameCustom(0)
{
	_image = tSharedImage(new Image(fileName, bAlpha));
	//_nFrames curr set to 0 in case Image class not initialised,
	//properly (due to bad image file etc), so now if it did init properly,
	//set the max number of frames, as passed in to this ctor
	setFrameCount(nFrames);
}

ImageAnim::ImageAnim(tSharedImage &img) :
	_image(img),
	_x(0.0f), _y(0.0f), _objectId(0),
	_frame(0), _firstFrame(0), _lastFrame(0), _nFrames(0), _frameDir(1),
	_repeat(1), _restart(0), _inflateBy(0),
	_visible(true), _pauseA(true), _rateA(0), _restartA(0),
	_delayA(0), _bDelayRestart(false), _frameCustom(0)
{
    setFrameCount(img->tileCount());
}

ImageAnim& ImageAnim::operator=(const ImageAnim &ia)
{
    // Check for self-assignment
    if (this != &ia)      // not same object
    {
        this->_image = ia._image; //tSharedImage(new Image(ia._image));

        this->_x = ia._x;
        this->_y = ia._y;
        this->_objectId = ia._objectId;
        this->_tileW = ia._tileW;
        this->_tileH = ia._tileH;
        this->_tileCount = ia._tileCount;
        this->_tileWOffset = ia._tileWOffset;
        this->_tileHOffset = ia._tileHOffset;
        this->_frame = ia._frame;
        this->_firstFrame = ia._firstFrame;
        this->_lastFrame = ia._lastFrame;
        this->_nFrames = ia._nFrames;
        this->_frameDir = ia._frameDir;
        this->_animType = ia._animType;
        this->_repeat = ia._repeat;
        this->_restart = ia._restart;
        this->_inflateBy = ia._inflateBy;
        this->_visible = ia._visible;
        this->_pauseA = ia._pauseA;
        this->_rateA = ia._rateA;
        this->_waitA = ia._waitA;
        this->_restartA = ia._restartA;
        this->_delayA = ia._delayA;
        this->_bDelayRestart = ia._bDelayRestart;
        this->_animCustom = ia._animCustom;
        this->_frameCustom = ia._frameCustom;
    }
    return *this;
}

//Code constructing an ImageAnim without an image can call this to pass in a resource image
//and apply all setup required before use.
void ImageAnim::setImage(tSharedImage &img)
{
    if (img.get())
    {
        _image = img;
        setFrameCount(img->tileCount());
    }
}

void ImageAnim::setFrameCount(Uint32 nFrames)
{
    if (_image->initDone())
    {
        _image->setTileCount(nFrames, _image->tileDir());
        _nFrames = _image->tileCount();
    }
}


void ImageAnim::setFrameLast()
{
    _frame = (_nFrames > 0)?_nFrames-1:0;   //force to last frame (so caller not need to know nFrames)
}

void ImageAnim::clearAnimCustom()
{
    _animCustom.clear();
}

//add single frame to custom list
bool ImageAnim::addAnimCustom(Uint32 frame)
{
    if (frame < _nFrames)
    {
        _animCustom.push_back(frame);
        return true;
    }
    return false;
}

//add a range. use overloaded sig as default param difficult to ignore with Uint
bool ImageAnim::addAnimCustom(Uint32 frameFrom, Uint32 frameTo)
{
    Uint32 i = frameFrom;
    if (frameTo > frameFrom)
    {
        while (i <= frameTo) addAnimCustom(i++);
    }
    else
    {
        while (i >= frameTo) addAnimCustom(i--);
    }
    return true;
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
    if (animType == ANI_CUSTOM)
    {
        if (_animCustom.empty()) return;    //fail - setAnimCustom() not called yet

        _frameCustom = 0;
        setFrame(_animCustom[_frameCustom]);
    }
    else
    {
        setFrameRange((firstFrame_0<0)?0:firstFrame_0, (lastFrame_N<0)?_nFrames-1:lastFrame_N);
        setFrame(firstFrame_0);
    }

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
	setAnimDir((_firstFrame < _lastFrame)?DIR_FORWARD:DIR_BACKWARD);
}

void ImageAnim::setFrame(Uint32 frame)
{
	_frame = (frame < _nFrames)?frame:0;	//must be valid frame num
}

void ImageAnim::setAnimType(eAnim animType)
{
	//set up the work function pointer depending on the type
    _animType = animType;
	switch (animType)
	{
	    case ANI_ONCEDEL:
		case ANI_ONCEPAUSE:
		case ANI_ONCEHIDE:
			pWorkFn = &ImageAnim::workONCE;		//iterate once then del/pause/hide
			break;
		case ANI_LOOP:
			pWorkFn = &ImageAnim::workLOOP;		//iterate & repeat
			break;
		case ANI_REVERSE:
			pWorkFn = &ImageAnim::workREVERSE;	//iterate then reverse iterate (repeat)
			break;
        case ANI_CUSTOM:
            pWorkFn = &ImageAnim::workCUSTOM;   //custom frame list, any order
            break;
		default:
			pWorkFn = &ImageAnim::workNONE;		//static, not moving/animating
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
			if (_animType == ANI_ONCEHIDE) setVisible(false);
			_sigEvent2((_animType == ANI_ONCEDEL)?USER_EV_END_DELETE:USER_EV_END_ANIMATION, _objectId);
			return;
		}
	}
	else
	{
		//subtracting to get prev frame
		if (_frame <= _firstFrame)
		{
			pauseAnim();
			if (_animType == ANI_ONCEHIDE) setVisible(false);
			_sigEvent2((_animType == ANI_ONCEDEL)?USER_EV_END_DELETE:USER_EV_END_ANIMATION, _objectId);
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
			_sigEvent2(USER_EV_END_ANIMATION, _objectId);
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
			_sigEvent2(USER_EV_END_ANIMATION, _objectId);
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
			_sigEvent2(USER_EV_END_ANIMATION, _objectId);
		}
	}
	else
	{
		//subtracting to get prev frame, if reached first frame, reverse dir
		if (_frame <= _firstFrame)
		{
			if (_repeat && _repeat-- <= 1) { pauseAnim(); return; }
			_frameDir = 1;
			_sigEvent2(USER_EV_END_ANIMATION, _objectId);
		}

	}
	_frame += _frameDir;
}

void ImageAnim::workCUSTOM(void)
{
    if (_frameCustom >= (Uint32)_animCustom.size())
    {
        pauseAnim();
		_sigEvent2(USER_EV_END_ANIMATION, _objectId);
        return;
    }
    _frame = _animCustom[_frameCustom++];
}

void ImageAnim::draw(Screen *s)
{
	if (_visible)
		blitTo(s, (int)round(_x), (int)round(_y), _frame);	//blit current frame to s at x, y
}

//add or subtract a border around the tile size border
//default 0 (same size as image tile)
void ImageAnim::setBounds(int inflateBy)
{
    _inflateBy = inflateBy;
    assert(inflateBy >= 0 || //positive, 0 or
           (abs(inflateBy) < std::max(tileW(), tileH())));    //less than max side if negative
}
//always size of the image tile in its current position with inflate amount prev set
Rect ImageAnim::bounds() const
{
    Rect r(_x, _y, _x+tileW(), _y+tileH());
    if (_inflateBy)
        return r.inset(_inflateBy);
    return r;
}


////return a new image from a tile in this image
//Image * Image::createImageFromThis(int tileNum, int iAlpha /*=-1*/)
//{
//	SDL_Rect r = tileRect(tileNum, _tileW, _tileH);
//	Image *image = new Image(r.w, r.h, iAlpha);
//	image->blitFrom(this, tileNum);
//	image->setTileSize(_tileW, _tileH);
//	return image;
//}

//create this image from another image (tile)
//void ImageAnim::createThisFromImage(Image &image, int tileNum /*=-1*/, int iAlpha /*=-1*/)
//{
//    _image->createThisFromImage(image, r, iAlpha);
//
//
//
//	cleanUp();
//	_init = Surface::create(image.tileW(), image.tileH(), iAlpha);
//
//	if (image.surface()->format->Amask && iAlpha!=-1)
//		//source image has alpha so set alpha in this new dest image too
//		SDL_SetAlpha(this->_surface, SDL_SRCALPHA, iAlpha);
//	else
//		//prefill with alpha colour so the final surface contains it where curr see through
//		ppg::drawSolidRect(this->_surface, 0, 0, image.tileW(), image.tileH(), ALPHA_COLOUR);
//	if (-1 == tileNum)
//		setTileSize();
//	else
//		setTileSize(image.tileW(), image.tileH());
//	blitFrom(&image, tileNum);	//into "this" newly created 'copy'
//}

//helper blit functions specifically for the Image class
//
//blit this (tile) image into another Image (or screen)
void ImageAnim::blitTo(Screen* dest, int destX, int destY, int tileNum /*= -1*/)
{
	//Image class objects default to clip = 0 unless explicitly set
	SDL_Rect rect = _image->tileRect(tileNum);
//	ppg::blit_surface(
//        _image->surface(), (tileNum<0)?nullptr:&rect,			//source
//		dest->surface(), destX, destY);					//dest
	dest->blit(_image->texture(), &rect, destX, destY);
}

//blit a whole image into this Image
void ImageAnim::blitFrom(Image* source, int destX, int destY )
{
	//Image class objects default to clip = 0 unless explicitly set
//	ppg::blit_surface(
//        source->surface(), nullptr,           	    //source
//		_image->surface(), destX, destY);		//dest
}

//blit a (tile) image into this Image
void ImageAnim::blitFrom(ImageAnim* source, int tileNum /*= -1*/, int destX, int destY )
{
	//Image class objects default to clip = 0 unless explicitly set
//	SDL_Rect rect = source->tileRect(tileNum);
//	ppg::blit_surface(
//        source->surface(), (tileNum<0)?nullptr:&rect,	    //source
//		_image->surface(), destX, destY);					//dest
}





