////////////////////////////////////////////////////////////////////
/*

File:			sprite.cpp

Class impl:		Sprite

Description:	A Image based class to handle specific movement of sliding
				a Image into or out of the screen. i.e. transition effects

				Surface->Image->ImageAnim->Sprite

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			14 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3.1	09.06.2007	set _xEnd to x and _yEnd to y in setPos(), was not clearing isMoving test
				0.5.0	18.06.2008	Added touch screen support

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
#include "sprite.h"
#include "screen.h"
#include "utils.h"
#include <math.h>


Sprite::Sprite() :
	ImageAnim(),
	_xStart(0), _yStart(0), _xEnd(0), _yEnd(0),
	_xDir(0), _yDir(0), _xVel(0), _yVel(0), _type(Sprite::SPR_NONE),
	_pauseM(true), _loopM(false), _rateM(0), _waitM(0), _touchable(true),
	_bEase(false)
{
}

Sprite::Sprite(std::string fileName, bool bAlpha, Uint32 nFrames) :
	ImageAnim(fileName, bAlpha, nFrames),
	_xStart(0), _yStart(0), _xEnd(0), _yEnd(0),
	_xDir(0), _yDir(0), _xVel(0), _yVel(0), _type(Sprite::SPR_NONE),
	_pauseM(true), _loopM(false), _rateM(0), _waitM(0), _touchable(true),
	_bEase(false)
{
}

Sprite::Sprite(tSharedImage &img) :
	ImageAnim(img), //constructs underlying Image member
	_xStart(0), _yStart(0), _xEnd(0), _yEnd(0),
	_xDir(0), _yDir(0), _xVel(0), _yVel(0), _type(Sprite::SPR_NONE),
	_pauseM(true), _loopM(false), _rateM(0), _waitM(0), _touchable(true),
	_bEase(false)
{
}

Sprite& Sprite::operator=(const Sprite &s)
{
    // Check for self-assignment
    if (this != &s)      // not same object
    {
        this->ImageAnim::operator=(s);

        this->_xStart = s._xStart;
        this->_yStart = s._yStart;
        this->_xEnd= s._xEnd;
        this->_yEnd = s._yEnd;
        this->_xDir = s._xDir;
        this->_yDir = s._yDir;
        this->_xVel = s._xVel;
        this->_yVel = s._yVel;
        this->_type = s._type;
        this->_points = s._points;
        //	tPoints::const_iterator	_pointit;
        this->_pauseM = s._pauseM;
        this->_loopM = s._loopM;
        this->_rateM = s._rateM;
        this->_waitM = s._waitM;
        this->_touchable = s._touchable;
    }
    return *this;
};

void Sprite::setPos(float x, float y)
{
	//movement positions
	_xStart = _xEnd = (int)round(x);	//save in case repeat or reverse attribute used
	_yStart = _yEnd = (int)round(y);	//Also, end pos = start to prevet isMoving()==true

	ImageAnim::setPos(x,y);
}

//set up move to without specifying velocities (just time, ie speed to get there)
//so we need to call calcXYRate to return the values required
void Sprite::startMoveTo(int xEnd, int yEnd, Uint32 time, Uint32 delay)
{
	float xVel, yVel;
	Uint32 rate = calcXYRate(time, xEnd, yEnd, (int)getXPos(), (int)getYPos(), xVel, yVel);
	startMoveTo(xEnd, yEnd, rate, delay, xVel, yVel, SPR_NONE);
}

void Sprite::easeMoveTo(int xEnd, int yEnd, Uint32 duration, Sint32 delay, Easing::eType ease)
{
	//direction
	int xDiff = 0;
    //set by end x or y, or overridden by SPR_ direction flags
    if ((xEnd < getXPos()))
    {
        _xDir = -1;
        xDiff = -(getXPos() - xEnd);
    }
    else if ((xEnd > getXPos()))
    {
        _xDir = 1;	//pos=right, neg=left
        xDiff = xEnd - getXPos();
    }
    else
        _xDir = 0;

	int yDiff = 0;
    if ((yEnd < getYPos()))
    {
        _yDir = -1;
        yDiff = -(getYPos() - yEnd);
    }
    else if ((yEnd > getYPos()))
    {
        _yDir = 1;	//pos=down, neg=up
        yDiff = yEnd - getYPos();
    }
    else
        _yDir = 0;

	_xEnd = xEnd;
	_yEnd = yEnd;

    _easeX.setup(ease, 0, getXPos(), xDiff, duration);
    _easeY.setup(ease, 0, getYPos(), yDiff, duration);

	//unpause movement if movement is set up correctly
	pauseMove( false );		//set true to allow moving

    if (delay < 0)
        delay = g_randInt.random(abs(delay));   //delay between 0-n where abs(delay) is max
    _waitM.start(0, delay);
	setMoveLoop(false);	 //not repeat any timer/movement

	_bEase = true;
}

//start move from A (curr pos) to B (to end) then stop or reverse etc.
//Pass SM_NONE to x or y to start from the current position,
//even before it has finished moving
void Sprite::startMoveTo(int xEnd, int yEnd,
						 Uint32 rate, Uint32 delay,
						 float xVel, float yVel,
						 eSprite type /*Sprite::SPR_NONE*/)	//repeat, reverse etc
{
	//direction
    //set by end x or y, or overridden by SPR_ direction flags
    if ((SPR_LEFT & type) || (xEnd < getXPos()))
        _xDir = -1;
    else if ((SPR_RIGHT & type) || (xEnd > getXPos()))
        _xDir = 1;	//pos=right, neg=left
    else
        _xDir = 0;

    if ((SPR_UP & type) || (yEnd < getYPos()))
        _yDir = -1;
    else if ((SPR_DOWN & type) || (yEnd > getYPos()))
        _yDir = 1;	//pos=down, neg=up
    else
        _yDir = 0;

	//end position
	//if SPR_ direction flags used (ie. no end pos), then determine off
	//screen end positions from sprite size, ie. sprite must be completely
	//off screen then stop (or reverse if that flag is set
	_xEnd = xEnd;
	_yEnd = yEnd;

	if (SPR_LEFT & type)
		_xEnd = -(tileW()+1); //calc end x pos (off screen)

	if (SPR_RIGHT & type)
		_xEnd = Screen::width() + 1; //calc end x pos (off screen)

	if (SPR_UP & type)
		_yEnd = -(tileH()+1); //calc end x pos (off screen)

	if (SPR_DOWN & type)
		_yEnd = Screen::height()+1; //calc end x pos (off screen)

	//velocity
	_xVel = xVel;
	_yVel = yVel;

//	calcWaypoints(_x, _y, xEnd, yEnd);

	//unpause movement if movement is set up correctly
	pauseMove( !canMove() );		//set false, if moving set
	setMoveLoop(true);				//repeat any timer/movement

	//finally start the timer if requested
	if (rate && canMove()) setMoveRate(rate, delay);

    _bEase = false;
}


void Sprite::work()
{
    if (_bEase) //use easing not linear
    {
        if (!_pauseM && _waitM.done(_loopM))
        {
            if (_xDir && !_easeX.done()) _x = _easeX.work();
            if (_yDir && !_easeY.done()) _y = _easeY.work();

            //check if end of move reached
            if ((!_xDir || ((_xDir>0 && _x >= _xEnd) || (_xDir<0 && _x <= _xEnd)) || (_easeX.done())) &&
                (!_yDir || ((_yDir>0 && _y >= _yEnd) || (_yDir<0 && _y <= _yEnd)) || (_easeY.done())) )
            {
                _x = _xEnd;	//make sure it stops at the exact end point given
                _y = _yEnd;
                pauseMove();

                //some use sig/slot
                _sigEvent2(USER_EV_END_MOVEMENT, _objectId);
                //others use SDL event  notification
                ppg::pushSDL_Event(USER_EV_END_MOVEMENT, reinterpret_cast<void *>(_objectId), 0);
            }
        }
    }
    else
    {
        //update position etc
        if (!_pauseM && _waitM.done(_loopM))
        {

            _x += _xVel*_xDir;
            _y += _yVel*_yDir;

            //check if end of move reached
            if ((!_xDir || ((_xDir>0 && _x >= _xEnd) || (_xDir<0 && _x <= _xEnd))) &&
                (!_yDir || ((_yDir>0 && _y >= _yEnd) || (_yDir<0 && _y <= _yEnd))) )
            {
                _x = _xEnd;	//make sure it stops at the exact end point given
                _y = _yEnd;
                pauseMove();

                //some use sig/slot
                _sigEvent2(USER_EV_END_MOVEMENT, _objectId);
                //others use SDL event  notification
                ppg::pushSDL_Event(USER_EV_END_MOVEMENT, reinterpret_cast<void *>(_objectId), 0);
            }

//            if (_pointit != _points.end())
//            {
//                _x = (*_pointit)._x;
//                _y = (*_pointit)._y;
//                ++_pointit;	//next waypoint
//            }
//            else
//            {
//                //finished all waypoints
//                pauseMove();
//            }
        }
    }

	ImageAnim::work();	//call parent to do the animation work
}

//if this sprite currently visible and clicked on return true
bool Sprite::contains(const Point &pt) const
{
	if (!isTouchable()) return false;
	Point thisPt((int)getXPos(), (int)getYPos());
	Rect r(thisPt, thisPt.add(Point(tileW(), tileH())));
	return r.contains(pt);
}

void Sprite::calcWaypoints(int x1, int y1, int x2, int y2)
{
    int dx, dy;		//delta
	int inx, iny;	//increment
	int e;			//

	_points.clear();

    dx = x2 - x1;
    dy = y2 - y1;
    inx = dx > 0 ? 4 : -4;
    iny = dy > 0 ? 4 : -4;

    dx = abs(dx);
    dy = abs(dy);

    if(dx >= dy)
	{
		dy <<= 1;
		e = dy - dx;
		dx <<= 1;
//		while (x1 != x2)
		while (((inx>0)?(x1<x2):(x1>x2)))
		{
			Point point(x1,y1);
			_points.push_back(point);
			if(e >= 0)
			{
				y1 += iny;
				e -= dx;
			}
			e += dy;
			x1 += inx;
		}
    }
	else
	{
		dx <<= 1;
		e = dx - dy;
		dy <<= 1;
//		while (y1 != y2)
		while (((iny>0)?(y1<y2):(y1>y2)))
		{
			Point point(x1,y1);
			_points.push_back(point);
			if(e >= 0)
			{
				x1 += inx;
				e -= dy;
			}
			e += dx;
			y1 += iny;
		}
    }
	Point point(x2,y2);	//last point must be exact end position wanted
	_points.push_back(point);

	_pointit = _points.begin();	//set iterator at start of list
}

//distance between two points
int Sprite::calcDistance(int x1, int y1, int x2, int y2)
{
	float dx = x2 - x1;
	float dy = y2 - y1
;
	float distance=sqrt(dx*dx + dy*dy);

	return abs((int)round(distance));
}


//return the delta x and y velocities (always positive - as dir gives +/-)
//depending on number of frames to show animation in
void Sprite::calcXYSteps(int frames, int x1, int y1, int x2, int y2, float &deltaX, float &deltaY)
{
	//dividing the vertical distance, and the horizontal distance by the
	//number of frames you want to take to move to this point.  The result will be
	//the number of pixels vertical and horizontal you need to move per frame.

	float dx = abs(x2 - x1);
	float dy = abs(y2 - y1);
	if (frames)
	{
		deltaX = fabs(dx / frames);
		deltaY = fabs(dy / frames);
	}
	else
	{
		deltaX = dx;
		deltaY = dy;
	}
}

//return the delta x and y velocities (always positive - as dir gives +/-)
//depending on the time required to move the sprite to its destination
Uint32 Sprite::calcXYRate(Uint32 time, int x1, int y1, int x2, int y2, float &deltaX, float &deltaY)
{
	float dist = calcDistance(x1,y1, x2,y2);
	float rate = (dist)?(time / dist):time;
	float dx = abs(x2 - x1);
	float dy = abs(y2 - y1);
	if (rate)
	{
		deltaX = fabs(dx / rate);
		deltaY = fabs(dy / rate);
	}
	else
	{
		deltaX = dx;
		deltaY = dy;
	}

	return (Uint32)round(rate);	//milliseconds between start x1/y1 and end x2/y2
}

