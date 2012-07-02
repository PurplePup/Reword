//sprite.h

#ifndef _SPRITE_H
#define _SPRITE_H

#include "imageanim.h"
#include "easing.h"

#include <deque>




class Sprite : public ImageAnim
{
public:
    typedef std::deque<Point> tPoints;

	enum eSprite {	SPR_NONE = 0,		//static, not moving
					SPR_LEFT = 1,
					SPR_RIGHT = 2,
					SPR_UP = 4,
					SPR_DOWN = 8
				};

//    struct Ease
//    {
//        Ease() : t(0), b(0), c(0), d(0), s(0) {}
//        float t;  //time ms (usually 0)
//        float b;  //start pos
//        float c;  //total change? ie start pos + dist to end pos
//        float d;  //duration ms
//        float s;
//    };
//
//    enum eEaseType { EASE_LINEAR, EASE_OUTBOUNCE };

	Sprite();
	Sprite(std::string fileName, bool bAlpha, Uint32 nFrames);
	Sprite(tSharedImage &img);
	virtual ~Sprite() {}

	Sprite& operator=(const Sprite &s);

	//start move from A to B then stop or repeat
	void startMoveTo(int xEnd, int yEnd, Uint32 time = 100, Uint32 delay = 0);
	void startMoveTo(int xEnd, int yEnd,
					Uint32 rate, Uint32 delay,
					float xVel, float yVel,
					eSprite type = Sprite::SPR_NONE);
	void calcWaypoints(int x1, int y1, int x2, int y2);
    void easeMoveTo(int xEnd, int yEnd, Uint32 duration, Sint32 delay, Easing::eType = Easing::EASE_OUTBOUNCE);

	int calcDistance(int x1, int y1, int x2, int y2);
	void calcXYSteps(int frames, int x1, int y1, int x2, int y2, float &deltaX, float &deltaY);
	Uint32 calcXYRate(Uint32 time, int x1, int y1, int x2, int y2, float &deltaX, float &deltaY);

	void setTouchable(bool b)		{ _touchable = b; }
	bool isTouchable() const		{ return _touchable; }

	void setMoveRate(Uint32 rate, Uint32 delay)	{ _waitM.start(_rateM = rate, delay); }
	void setMoveLoop(bool loop)		{ _loopM = loop;}			//keep moving each time?
	void pauseMove(bool b = true)	{ _pauseM = b; }			//stop/start moving
	void toggleMove()				{ _pauseM = !_pauseM; }		//toggle pause state
	bool canMove() const            { return ((_xVel || _yVel) && (_xDir || _yDir)); }
	bool isMoving() const       	{ return !(_x == _xEnd && _y == _yEnd); }	//has it reached end point yet
	int  getXEnd() const            { return _xEnd; }
	int  getYEnd() const            { return _yEnd; }

	//overridden functions
	virtual void setPos(float x, float y);
	virtual void work();	//update movement, and call parent anim update
	virtual bool contains(const Point &pt) const;

protected:

	int		_xStart;	//save start x pos
	int		_yStart;	//save start y pos
	int		_xEnd;		//ending x position
	int		_yEnd;		//ending y position

	int		_xDir;		//pos=right, neg=left
	int		_yDir;		//pos=down, neg=up
	float	_xVel;		//amount of change in x dir
	float	_yVel;		//amount of change in y dir

	eSprite _type;		//predefined movement etc

	tPoints	_points;
	tPoints::const_iterator	_pointit;

private:
	bool	_pauseM;	//true if not moving (default=false)
	bool	_loopM;		//loop the movement (default=true), or just once through
	Uint32	_rateM;		//rate of auto movement change
	Waiting _waitM;		//movement delay
	bool	_touchable;

	bool    _bEase;
	Easing  _easeX, _easeY;

};

typedef boost::shared_ptr<Sprite> t_pSharedSpr;

#endif //_SPRITE_H


