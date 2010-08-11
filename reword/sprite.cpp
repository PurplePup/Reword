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
#include <math.h>


Sprite::Sprite() : 
	ImageAnim(), 
	_xStart(0), _yStart(0), _xEnd(0), _yEnd(0), 
	_xDir(0), _yDir(0), _xVel(0), _yVel(0), _type(Sprite::SPR_NONE),
	_pauseM(true),_loopM(false), _rateM(0), _waitM(0), _touchable(true)
{
}

Sprite::Sprite(std::string fileName, bool bAlpha, Uint32 nFrames) :
	ImageAnim(fileName, bAlpha, nFrames), 
	_xStart(0), _yStart(0), _xEnd(0), _yEnd(0), 
	_xDir(0), _yDir(0), _xVel(0), _yVel(0), _type(Sprite::SPR_NONE),
	_pauseM(true),_loopM(false), _rateM(0), _waitM(0), _touchable(true)
{
}

Sprite::Sprite(const Image &img) :
	ImageAnim(img), 
	_xStart(0), _yStart(0), _xEnd(0), _yEnd(0), 
	_xDir(0), _yDir(0), _xVel(0), _yVel(0), _type(Sprite::SPR_NONE),
	_pauseM(true),_loopM(false), _rateM(0), _waitM(0), _touchable(true)
{
}

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

//start move from A (curr pos) to B (to end) then stop or reverse etc.
//Pass SM_NONE to x or y to start from the current position, 
//even before it has finished moving
void Sprite::startMoveTo(int xEnd, int yEnd, 
						 Uint32 rate, Uint32 delay,  
						 float xVel, float yVel,
						 eSprite type /*Sprite::SPR_NONE*/)	//repeat, reverse etc
{
	//direction
	//set by end x or y, or overridden by SM_ direction flags
	if ( (SPR_LEFT & type) || (xEnd < getXPos()) ) 
		_xDir = -1; else _xDir = 1;	//pos=right, neg=left
	if ( (SPR_UP & type) || (yEnd < getYPos()) )
		_yDir = -1; else _yDir = 1;	//pos=down, neg=up

	//end position
	//if SM_ direction flags used (ie. no end pos), then determine off
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

	//attributes
//##TODO## effects flags stuff here

//	calcWaypoints(_x, _y, xEnd, yEnd);


	//unpause movement if movement is set up correctly
	pauseMove( !canMove() );		//set false, if moving set
	setMoveLoop(true);				//repeat any timer/movement

	//finally start the timer if requested
	if (rate && canMove()) setMoveRate(rate, delay);

}

void Sprite::work()
{
	//update position etc
	if (!_pauseM && _waitM.done(_loopM))
	{

		_x += _xVel*_xDir;
		_y += _yVel*_yDir;

		//check if end of move reached
		if ((_xDir != 0 && ((_xDir>0 && _x >= _xEnd) || (_xDir<0 && _x <= _xEnd))) &&
			(_yDir != 0 && ((_yDir>0 && _y >= _yEnd) || (_yDir<0 && _y <= _yEnd))) )
		{
			_x = _xEnd;	//make sure it stops at the exact end point given
			_y = _yEnd;
			pauseMove();
		}

/*
		if (_pointit != _points.end())
		{
			_x = (*_pointit)._x;
			_y = (*_pointit)._y;
			++_pointit;	//next waypoint
		}
		else
		{
			//finished all waypoints
			pauseMove();
		}
*/
	}

	ImageAnim::work();	//call parent to do the animation work
}

//if this sprite currently visible and clicked on return true
bool Sprite::contains(Point pt)
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

