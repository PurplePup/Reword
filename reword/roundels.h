//roundels.h


#ifndef _ROUNDELS_H
#define _ROUNDELS_H

#include <string>
#include <map>
#include <vector>

#include "sprite.h"
#include "states.h"

class Roundels
{
public:

	class Roundel
	{
	public:
		Roundel() : _letter(0), _pos(0), _spr(0) {}
		~Roundel() { delete _spr; }
		char		_letter;
		int			_pos;
		Sprite *	_spr;
	};

	Roundels();
	~Roundels();
	void cleanUp();
	void setWord(std::string &wrd, Image &letters, int x /*= 0*/, int y /*= 0*/, int gap /*= 0*/, bool bHoriz /*= true*/);
	void setWordCenterHoriz(std::string wrd, Image &letters, int y = 0, int gap =0);
	void setWordCenterVert(std::string wrd, Image &letters, int x = 0, int gap =0);
	void startMoveFrom(int deltaX, int deltaY, Uint32 rate, Uint32 delay, int xVel, int yVel, Sprite::eSprite type = Sprite::SPR_NONE);
	void setTopAndBottomYPos(int yPosTop, int yPosBot);

	bool isInOrder();
	bool jumbleWord(bool bAnimate = true);
	bool unJumbleWord(bool bAnimate = true);
	void moveLetterUp();
	void moveLetterDown();
	bool cursorFirst();
	bool cursorLast();
	bool cursorPrev();
	bool cursorNext();
	void cursorUp();
	void cursorDown();
	bool cursorAt(Point p);	// return false if p isn't in a valid roundel

	std::string getBottomWord();
	int getBottomWordLength() { return _botLength; }

	void clearAllToTop(bool bResetCursor = true);
	void setWordToLast();

	void draw(Surface *s);
	void work();
	bool isMoving() {return _bMoving;}

	bool cursorIsTop() const { return _bCursorTop; }
	int  currentX() const { return _cx; }

	int getX() const { return _x; }	//screen x pos of first roundel
	int getY() const { return _y; }	//screen y ...

protected:

	void recalcXYPosition(Roundel *r);
	int calcXPos(Roundel *r);
	int calcYPos(Roundel *r);

	typedef std::vector<Roundel*>	tRoundVect;
	typedef std::vector<int>		tLastVect;

	tRoundVect	_top;
	tRoundVect	_bot;
	tRoundVect	_last;			//letters of last word (only uses copies of pointers)

	int		_x, _y, _gap;		//screen position of first (top) roundel, saved from setWord
	bool	_bHoriz;
	bool	_bMoving;			//indicates if all sprites have stopped moving or not

	std::string	_word;			//original word used to set roundels

	int _cx;					//cursor (highlight) x and y pos
	int _botLength;             //current count of actual letters in bottom row
	bool _bCursorTop;			//cursor curr on top row or bottom row?

	int _yScratchTop,
		_yScratchBot;		//previously #defined SCRATCHY1 and SCRATCHY2
};

#endif //_ROUNDELS_H
