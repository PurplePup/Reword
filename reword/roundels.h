//roundels.h

#ifndef _ROUNDELS_H
#define _ROUNDELS_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "sprite.h"
#include "states.h"
#include "i_play.h"

class Roundel
{
public:
    Roundel() : _letter(0), _pos(0), _spr(0) {}
    ~Roundel() { delete _spr; }

    Roundel& operator=(const Roundel &r);

    char		_letter;
    int			_pos;
    Sprite *	_spr;
};

class Roundels : IPlay
{
public:

	Roundels();
	~Roundels();
	void cleanUp();
	void setWord(const std::string &wrd, tSharedImage &letters, int x = 0, int y = 0, int gap = 0, bool bHoriz = true);
	void setWordCenterHoriz(const std::string &wrd, tSharedImage &letters, int y = 0, int gap =0);
	void setWordCenterVert(const std::string &wrd, tSharedImage &letters, int x = 0, int gap =0);
	void startMoveFrom(int deltaX, int deltaY, Uint32 rate, Uint32 delay, int xVel, int yVel, Sprite::eSprite type = Sprite::SPR_NONE);
    void easeMoveFrom(int deltaX, int deltaY, Uint32 duration, Uint32 delay, Easing::eType ease = Easing::EASE_OUTBOUNCE);
	void setBottomPos(int xPosBot, int yPosBot);
	Point getBottomPos();
	void setBottomMax(int iMax);
	void setBottomCopy(bool bCopyBot = true) { _bCopyBot = bCopyBot; }
    int getLastId();
    int getCurrSelId();
    Point getCurrSelPt();

    void setPressEffect(const std::string &resource);

    void setRoundelsId(int rid) { _rid = rid; }
    int getRoundelsId() { return _rid; }

	bool isInOrder();
	bool jumbleWord(bool bAnimate = true);
	bool unJumbleWord(bool bAnimate = true);
	bool moveLetterUp(bool bEffect = true);
	bool moveLetterDown(bool bEffect = true);
	bool cursorFirst();
	bool cursorLast();
	bool cursorPrev();
	bool cursorNext();
	void cursorUp();
	void cursorDown();
	int cursorAt(Point p);	// return 0 if p isn't in a valid roundel else roundel No+1

	std::string getBottomWord();
	int getBottomWordLength() { return _botLength; }

	void clearAllToTop(bool bResetCursor = true);
	void setWordToLast();

	bool isMoving() {return _bMoving;}

	bool cursorIsTop() const { return _bCursorTop; }
	int  currentX() const { return _cx; }

	int getX() const { return _xScratchTop; }	//screen x pos of first roundel
	int getY() const { return _yScratchTop; }	//screen y ...

    int getRoundelW() { return _roundelW; }
    int getRoundelH() { return _roundelH; }

    //init the level/screen
    virtual void init(Input * /*input*/);
    // drawing operation
    virtual void render(Screen* s);
    // other processing
    virtual void work(Input* input, float speedFactor);
    // notification of button/input state change
    virtual void button(Input* /*input*/, ppkey::eButtonType /*b*/);

	// screen touch (press)
	virtual bool touch(const Point &pt);
	// screen touch (release)
	virtual bool tap(const Point &pt);


protected:

	void recalcXYPosition(Roundel *r);
	int calcXPos(Roundel *r);
	int calcYPos(Roundel *r);
    void slotEvent(int event, int id);

	typedef std::vector<Roundel*>	tRoundVect;
	typedef std::vector<int>		tLastVect;

	tRoundVect	_top;
	tRoundVect	_bot;
	tRoundVect	_last;			//letters of last word tried (only uses copies of pointers)

	std::string	_word;			//original word used to set roundels

    std::string _pressResource;
    int         _pressEffW, _pressEffH;

    //positioning...
	int     _xScratchTop, _yScratchTop,
            _xScratchBot, _yScratchBot;		//previously #defined SCRATCHY1 and SCRATCHY2
    int     _roundelW, _roundelH;

	int		_gap;       		//gap betwen letters on screen
	bool	_bHoriz;
	bool	_bMoving;			//indicates if all sprites have stopped moving or not

	int     _cx;				//cursor (highlight) x and y pos
	int     _botLength;         //current count of actual letters in bottom row
	int     _botLengthMax;
	bool    _bCursorTop;		//cursor curr on top row or bottom row?
    bool    _bCopyBot;          //copy to bottom rather than move (used in hiscore etc)

    int     _currSelId;         //last roundel touched (before tap/release)
    Point   _currSelPt;

    int     _lastIdCountdown;   //countdown to last roundel to stop moving
    int     _lastId;            //lat sprite object id for easy notification of end movement etc
    int     _rid;               //unique id for this roundels class implementation
};

typedef std::auto_ptr<Roundels> tAutoRoundels;

#endif //_ROUNDELS_H
