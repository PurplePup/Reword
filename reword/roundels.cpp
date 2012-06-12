////////////////////////////////////////////////////////////////////
/*

File:			roundels.cpp

Class impl:		Roundels

Description:	A class to manage a list of roundels (letter sprites) to animate and
				move them around. This is used for the screen titles where the letters
				drop from above the screen, and for the main 6 letter word used in the game
				which can be selected using the cursor and moved to the lower tiles for
				matching against the word(s) being searched for. The class manages top and
				bottom rows of letters but only the top row is used for simple letter
				animation. The bottom is only used in-game for the selected letters.
				Now also used for kbd selection in hiscore table entry.

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			18 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3.1	03.06.2007	Added destructor (D'oh!) to call cleanUp()
									Added ismoving test to fns to stop rogue letters flying
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
#include "roundels.h"
#include "screen.h"
#include "locator.h"

#include <algorithm>
#include <iostream>


//single Roundel copy
Roundel& Roundel::operator=(const Roundel &r)
{
    // Check for self-assignment
    if (this != &r)      // not same object
    {
        this->_letter = r._letter;
        this->_pos = r._pos;
        this->_spr = new Sprite(*r._spr);
    }
    return *this;
}

//now Roundels

Roundels::Roundels() :
    _xScratchTop(0), _yScratchTop(28), //legacy GP2X y positions
    _xScratchBot(0), _yScratchBot(73),
    _bCopyBot(false)
{
}

Roundels::~Roundels()
{
	cleanUp();
}

//init the level/screen
void Roundels::init(Input * /*input*/)
{
    cleanUp();
}

void Roundels::setPressEffect(const std::string &resource)
{
    //create a cursor anim here to auto remove on completion
    _pressResource = resource;
//    _pressEffect = t_pSprite(new Sprite(Resource::image(resource)));
    _pressEffW = Resource::image(resource)->tileW();
    _pressEffH = Resource::image(resource)->tileH();
//    _pressEffW = _pressEffect->tileW();
//    _pressEffH = _pressEffect->tileH();
}

// drawing operation
void Roundels::render(Screen* s)
{
	tRoundVect::iterator it;
	for (it = _top.begin(); it != _top.end(); ++it)
        if (*it)
            (*it)->_spr->draw(s);

	for (it = _bot.begin(); it != _bot.end(); ++it)
        if (*it)
            (*it)->_spr->draw(s);
}

// other processing
void Roundels::work(Input* input, float speedFactor)
{
    (void)(input);
    (void)(speedFactor);

	tRoundVect::iterator it;
	bool bMoving = false;
	for (it = _top.begin(); it != _top.end(); ++it)
	{
		if (*it) 	//letter exists in this top position
		{
			(*it)->_spr->work();
			bMoving |= (*it)->_spr->isMoving();
		}
	}

	for (it = _bot.begin(); it != _bot.end(); ++it)
	{
		if (*it) 	//letter exists in this bottom position
		{
			(*it)->_spr->work();
			bMoving |= (*it)->_spr->isMoving();
		}
	}

	_bMoving = bMoving;	//if any are moving, something/someone might need to know
}

// notification of button/input state change
void Roundels::button(Input* /*input*/, ppkey::eButtonType /*b*/)
{
}

// screen touch (press)
bool Roundels::touch(const Point &pt)
{
    _currSelId = cursorAt(pt);

    if (_currSelId)
    {
        if (cursorIsTop())
            _currSelPt = Point((int)_top[_currSelId-1]->_spr->getXPos(), (int)_top[_currSelId-1]->_spr->getYPos());
        else
            _currSelPt = Point((int)_bot[_currSelId-1]->_spr->getXPos(), (int)_bot[_currSelId-1]->_spr->getYPos());
    }
    else
        _currSelPt = Point();

    return _currSelId != 0;
}

// screen touch (release)
bool Roundels::tap(const Point &pt)
{
    if (_currSelId && (cursorAt(pt) == _currSelId))
    {
        if (cursorIsTop())
        {
            if (moveLetterDown())
            {
                Locator::audio().playSfx(AUDIO_SFX_ROUNDEL);
                cursorDown();
            }
        }
        else
        {
            if (moveLetterUp())
            {
                Locator::audio().playSfx(AUDIO_SFX_ROUNDEL);
                if (!cursorPrev())
                    cursorUp();
            }
        }
        _currSelId = 0;
        return true;
    }
    return false;
}

void Roundels::cleanUp()
{
	tRoundVect::iterator it;
	for (it = _top.begin(); it != _top.end(); ++it) if (*it) delete *it;
	_top.clear();

    if (!_bCopyBot)
        for (it = _bot.begin(); it != _bot.end(); ++it) if (*it) delete *it;
	_bot.clear();

	_botLength = _botLengthMax = 0;

	_last.clear();	//only uses a copy of _top or _bot pointers so doesnt delete

	_xScratchTop = _yScratchTop = _xScratchBot = _yScratchBot = 0;
	_bMoving = false;
	_currSelId=0;
	_currSelPt=Point();
    _lastIdCountdown = 0;
	_lastId = 0;
	_rid = 0;
}

//pass in the string to be turned into a roundel list and set the final
//positions of the letters based on the x, y and gap values
//wrd - the string to be converted to roundels,
//letters - the letters image to use as the source,
//x, y - the start x and y pos (if not 0,0)
//gap - and the distance between each letter...
//	horizontally or vartically (not both)
//	other more advanced positioning must be done manually using the
//	accessor functions after the setWord function returns.
//bHoriz - default true for horizontal letters, false for vertical
void Roundels::setWord(const std::string &wrd,
					   tSharedImage &letters,
					   int x, int y,
					   int gap,
					   bool bHoriz)
{
	cleanUp();
	_word = wrd;	//save for testing against in jumbleWord() etc

	//reset cursor
	_cx = 0; //left
	_bCursorTop = true;	//top

	_xScratchTop = x;
	_yScratchTop = y;
	_gap = gap;
	_bHoriz = bHoriz;
	_botLengthMax = _word.length();

	for(int i = 0; i < (int)wrd.length(); ++i)
	{
		//for each letter in string, add its roundel
		Roundel *prnd = new Roundel();
		Sprite *pspr = new Sprite(letters);

        //connect each sprite roundel to the event function slot
        pspr->_sigEvent2.Connect(this, &Roundels::slotEvent);

		pspr->setFrame(wrd[i]-65);

		//each letter has id of initial index (1..n), then use getLastId()
		//to determine the current last letter id.
        pspr->setObjectId((_lastId = i+1));

		prnd->_letter = wrd[i];	//wrd[i] - 65 == 0=a, 1=b
		prnd->_pos = i;			//letter position in string
		prnd->_spr = pspr;		//pointer to sprite

		//set up basic positions of each letter in the string
		recalcXYPosition(prnd);

		_top.push_back(prnd);

		//reuse ptr to build lists for _bot and _last
		//that currently dont point to anything
		prnd = (Roundel*)NULL;
		_bot.push_back(prnd);	//set to null
		_last.push_back(prnd);	//set to null
	}
}

//Event signal from imageanim indicating an individual sprite has
//finished animation or movement etc.
//Pass an event back (as SDL event) for higher 'IPlay' classes to detect.
void Roundels::slotEvent(int event, int id)
{
    if (event == USER_EV_END_MOVEMENT)
    {
        //indicate a single roundel has stopped moving
        ppg::pushSDL_Event(USER_EV_END_MOVEMENT,
                           reinterpret_cast<void *>(id),        //roundel sprite/item id
                           reinterpret_cast<void *>(_rid));     //this class instance id

        //if it's the last one to stop moving, send this message too
        if (--_lastIdCountdown== 0)
            ppg::pushSDL_Event(USER_EV_END_MOVEMENT_ROUNDEL,
                               reinterpret_cast<void *>(id),        //roundel sprite/item id
                               reinterpret_cast<void *>(_rid));     //this class instance id
    }
}

//return the id of the last roundel in the word (note: pre jumble)
int Roundels::getLastId()
{
    return _lastId;
}

//get the id of the currently selected/highlighted roundel
int Roundels::getCurrSelId()
{
    return _currSelId;
}

//return the position of the currently selected/highlighted roundel
Point Roundels::getCurrSelPt()
{
    return _currSelPt;
}


void Roundels::setBottomPos(int xPosBot, int yPosBot)
{
	_xScratchBot = xPosBot;
	_yScratchBot = yPosBot;
}

void Roundels::setBottomMax(int iMax)
{
    if (iMax > (int)_word.length())
        iMax = _word.length();
    _botLengthMax = iMax;
}

//center the roundels horizontally on screen (no need to specify x position)
void Roundels::setWordCenterHoriz(const std::string &wrd,
					   tSharedImage &letters,
					   int y,				//no x if centered horiz
					   int gap)
{
	//calc new start position of first roundel based on centered length of all roundels in the word
	const int newX = ( Screen::width() - (((letters->tileW()+gap)*wrd.length())-gap) ) /2;
	const int newY = y;

	//pass new pos to the real setWord function
	setWord(wrd, letters, newX, newY, gap, true);
}

//center the roundels vertically on screen (no need to specify y position)
void Roundels::setWordCenterVert(const std::string &wrd,
					   tSharedImage &letters,
					   int x,				//no y if centered vert
					   int gap)
{
	//calc new start position of first roundel based on centered length of all roundels in the word
	const int newX = x;
	const int newY = ( Screen::height() - (((letters->tileH()+gap)*wrd.length())-gap) ) /2;

	//pass new pos to the real setWord function
	setWord(wrd, letters, newX, newY, gap, false);
}

//move the roundels to a starting position (usually off screen) from their current start position
//stated in setWord() and move them back to their original position using speed and direction
//ie this fn should be called after setWord()
void Roundels::startMoveFrom(int deltaX, int deltaY,
							 Uint32 rate, Uint32 delay,
							 int xVel, int yVel,
							 Sprite::eSprite type /*= Sprite::SPR_NONE*/)
{
    _lastIdCountdown = (int)_word.length(); //reset to number of roundels

	int oldX, oldY;
	int i = 0;
	tRoundVect::iterator it;
	for (it = _top.begin(); it != _top.end(); ++it)
	{
		if (*it)
		{
			oldX = (int)((*it)->_spr->getXPos());
			oldY = (int)((*it)->_spr->getYPos());
			(*it)->_spr->setPos(deltaX+oldX, deltaY+oldY);
			(*it)->_spr->startMoveTo(oldX, oldY, rate, delay*i, xVel, yVel, type);
		}
		++i;
	}
}


//determine if the word (in _top) is in the correct order, ie. matches the target N letter _word
bool Roundels::isInOrder()
{
	int n = 0;
	tRoundVect::iterator it;
	for (it = _top.begin(); it != _top.end(); ++it)
	{
		if (!(*it) || (*it)->_letter != _word[n]) return false; //at least this letter not matching
		++n;
	}
	return true;
}

//jumble/randomize letters in top array
//If too few letters, exit
//If jumbled word matches original, loop again
//If loop again > 5 times, then exit - failsafe
bool Roundels::jumbleWord(bool bAnimate /*=true*/)
{
	if (isMoving()) return false;

	int xx = 0;		//pos to swap...
	int xx2 = 0;	//...with this pos
	Roundel *rtmp;

	//count letters in top array
	int nTop = 0;
	tRoundVect::iterator it;
	for (it = _top.begin(); it != _top.end(); ++it) if ((*it)) nTop++;
	if (nTop < 3) return false;	//dont bother jumbling, not enough letters

	int loops = 6;	//failsafe to exit after 5 retries,in case XXXXXX or other silly found
	do
	{
		xx=0;	//keep a position count in the vector as we access it using [xx]
		for (xx = 0; xx < (int)_top.size(); ++xx)
		{
			//dont swap blank spaces or letters currently moving up or down
			if (!(_top[xx]) || _top[xx]->_spr->isMoving()) continue;

			do
			{	//rand another letter position
				xx2 = (rand()%_top.size());	//0..5
				//xx2 = ((int)_gd._myrand.Random(6));	//0..5
			} while (xx2 == xx || !(_top[xx2]));

			//swap the actual pointers to the roundels
			//(makes it easier all round rather than trying to sort/determine positions)
			rtmp = _top[xx2];
			_top[xx2] = _top[xx];
			_top[xx] = rtmp;
			//now swap positioins
			_top[xx]->_pos = xx;
			_top[xx2]->_pos = xx2;
		}
		if (!isInOrder()) break;	//not spelling original word so ok, exit loop
	} while (loops--);	//check loops THEN decrement (as 'loops' can be set to 0 to exit)

    _lastIdCountdown = nTop; //reset to number of roundels in top row to jumble

	//recalc screen positions now letter positions have been jumbled
	for (it = _top.begin(); it != _top.end(); ++it)
	{
		if (*it && !(*it)->_spr->isMoving())
		{
			if (bAnimate)
				(*it)->_spr->startMoveTo(calcXPos((*it)), calcYPos((*it)), 25, 0, 9, 0);	//start slide anim
			else
				recalcXYPosition((*it));
		}
	}
	return true;
}

//unjumble/unrandomize letters in top array
//If too few letters, exit
//Only used in main menu screen on REWORD letters above the menu.
//NOTE: if it is used elsewhere, we may need to protect it aainst rogue letter stray
//		like the jumbleWord fn where ismoving() needed to be checked
bool Roundels::unJumbleWord(bool bAnimate /*=true*/)
{
	if (isMoving()) return false;

	int xx = 0;		//pos to swap...
	int xx2 = 0;	//...with this pos
	Roundel *rtmp;

	//count letters in top array
	int nTop = 0;
	tRoundVect::iterator it;
	for (it = _top.begin(); it != _top.end(); ++it) if ((*it)) nTop++;
	if (nTop < 3) return false;	//dont bother jumbling, not enough letters

	for (xx=0; xx<(int)_word.length(); ++xx)
	{
		if (_top[xx]->_letter == _word[xx]) continue;		//already matches

		//find a match for curr letter in remaining letters
		for (xx2=xx+1; xx2<(int)_word.length(); ++xx2)
		{
			if (_top[xx2]->_letter == _word[xx])
			{
				//swap xx and xx2
				rtmp = _top[xx];
				_top[xx] = _top[xx2];
				_top[xx2] = rtmp;
				//now swap positioins
				_top[xx]->_pos = xx;
				_top[xx2]->_pos = xx2;
			}
		}
	}

    _lastIdCountdown = nTop; //reset to number of roundels in top row to unjumble

	//recalc screen positions now letter positions have been jumbled
	for (it = _top.begin(); it != _top.end(); ++it)
	{
		if (*it)
		{
			if (bAnimate)
				(*it)->_spr->startMoveTo(calcXPos((*it)), calcYPos((*it)), 25, 0, 9, 0);	//start slide anim
			else
				recalcXYPosition((*it));
		}
	}
	return true;
}

//recalculate the sprites position on screen based on its internal
//_pos value within the list of letters making up the word
void Roundels::recalcXYPosition(Roundel *r)
{
	r->_spr->setPos( calcXPos(r), calcYPos(r) );
}
int Roundels::calcXPos(Roundel *r)
{
	return _xScratchTop+(r->_pos*((_bHoriz)?(r->_spr->tileW()+_gap):0));
}
int Roundels::calcYPos(Roundel *r)
{
	return _yScratchTop+(r->_pos*((!_bHoriz)?(r->_spr->tileH()+_gap):0));
}

//move the selected letter (_cx) to the first
//free space in the top row
bool Roundels::moveLetterUp(bool bEffect /*=true*/)
{
	//if letter already moved up or is moving so wait (as can cause undefined behaviour)
	if (isMoving() || !_bot[_cx]) return false;

    if (_bCopyBot)
    {
        const int id = _bot[_cx]->_spr->getObjectId();

        if (bEffect)
        {
            const int oldX = (int)(_bot[_cx]->_spr->getXPos());
            const int oldY = (int)(_bot[_cx]->_spr->getYPos());
            //create a cursor anim here to auto remove on completion
            const int x = oldX - (( _pressEffW - _bot[_cx]->_spr->tileW()) / 2);
            const int y = oldY - (( _pressEffH - _bot[_cx]->_spr->tileH()) / 2);
            Locator::data()._effects.add(_pressResource, x, y);  //add global effect
        }

        //need to remove from bottom, but leave top alone
        delete _bot[_cx];
        _bot[_cx] = 0;
        _botLength--;

        //notify parent single roundel has 'moved'
        ppg::pushSDL_Event(USER_EV_END_MOVEMENT,
            reinterpret_cast<void *>(id), reinterpret_cast<void *>(_rid));

        return true;
    }

	int xx;
	for (xx=0; xx < (int)_word.length(); ++xx)
	{
		if (0 == _top[xx])	//blank position so use it
		{
			const int oldX = (int)(_bot[_cx]->_spr->getXPos());
			const int oldY = (int)(_bot[_cx]->_spr->getYPos());

            if (bEffect)
            {
                //create a cursor anim here to auto remove on completion
                const int x = oldX - (( _pressEffW - _bot[_cx]->_spr->tileW()) / 2);
                const int y = oldY - (( _pressEffH - _bot[_cx]->_spr->tileH()) / 2);
                Locator::data()._effects.add(_pressResource, x, y);  //add global effect
            }

			_top[xx] = _bot[_cx];
			_top[xx]->_pos = xx;
			_bot[_cx] = 0;

			int endX = _xScratchTop+(xx*(_top[xx]->_spr->tileW()+_gap));
			int endY = _yScratchTop;

			float velX, velY;
			Uint32 rate = _top[xx]->_spr->calcXYRate(200, oldX, oldY, endX, endY, velX, velY);
			_top[xx]->_spr->startMoveTo(endX, endY, rate, 0, velX, velY);

            _botLength--;
			return true;
		}
	}
	return false;
}

//move the selected letter (_cx) to the first
//free space in the bottom row
bool Roundels::moveLetterDown(bool bEffect /*=true*/)
{
	//if letter already moved down or is moving so wait (as can cause undefined behaviour)
	if (isMoving() || !_top[_cx]) return false;
    if (_botLength >= _botLengthMax) return false;    //can't move any more

	int xx;
	for (xx=0; xx < (int)_word.length(); ++xx)
	{
		if (0 == _bot[xx])	//blank position so use it
		{
			int oldX = (int)(_top[_cx]->_spr->getXPos());
			int oldY = (int)(_top[_cx]->_spr->getYPos());

            if (bEffect)
            {
                //create a cursor anim here to auto remove on completion
                const int x = oldX - (( _pressEffW - _top[_cx]->_spr->tileW()) / 2);
                const int y = oldY - (( _pressEffH - _top[_cx]->_spr->tileH()) / 2);
                Locator::data()._effects.add(_pressResource, x, y);  //add global effect
            }

            if (_bCopyBot)
            {
                //duplicate (copy) sprite
                Roundel *r = new Roundel();
                *r = *(_top[_cx]);
                _bot[xx] = r;
            }
            else
            {
                _bot[xx] = _top[_cx];
                _bot[xx]->_pos = xx;
                _top[_cx] = 0;
			}

            int endX = _xScratchBot+(xx*(_bot[xx]->_spr->tileW()+_gap));
            int endY = _yScratchBot;

            float velX, velY;
            Uint32 rate = _bot[xx]->_spr->calcXYRate(400, oldX, oldY, endX, endY, velX, velY);
            _bot[xx]->_spr->startMoveTo(endX, endY, rate, 0, velX, velY);

            _botLength++;
			return true;
		}
	}
	return false;
}

//move the cursor to the first non space (on either top or bottom row)
bool Roundels::cursorFirst()
{
	int i = 0;
	while (i < (int)_word.length())
	{
		if (0 != (_bCursorTop?_top[i]:_bot[i]))
		{
			_cx = i;
			return true;
		}
		++i;
	}
	return false;	//no letters
}

//move the cursor to the last nonspace (top or bottom row)
bool Roundels::cursorLast()
{
	int i = _word.length()-1;
	while (i >= 0)
	{
		if (0 != (_bCursorTop?_top[i]:_bot[i]))
		{
			_cx = i;
			return true;
		}
		--i;
	}
	return false;	//no letters
}

//move the cursor to the next non space to the left
bool Roundels::cursorPrev()
{
	if (!_bCursorTop) return false; //not move on bottom row
	const int save = _cx;
	int i = _cx;
	do {
		if (i == 0) i = _word.length()-1; else --i;
		if (0 != (_bCursorTop?_top[i]:_bot[i]))
		{
			_cx = i;
			return true;
		}
	} while (i != save);
	return false;	//no letters
}

//move the cursor to the next non space to the right
bool Roundels::cursorNext()
{
	if (!_bCursorTop) return false; //not move on bottom row
	const int save = _cx;
	int i = _cx;
	do {
		if (i == (int)_word.length()-1) i = 0; else ++i;
		if (0 != (_bCursorTop?_top[i]:_bot[i]))
		{
			_cx = i;
			return true;
		}
	} while (i != save);
	return false;	//no letters
}

//move cursor to top row - finding next non space
void Roundels::cursorUp()
{
   	_bCursorTop = true;
	if (_top[_cx] == 0) //back on top, position is space so find next
	{
		if (!cursorNext())
		{
			_bCursorTop = false;	//back to bottom
		}
	}
}

//move the cursor down to the last non space
void Roundels::cursorDown()
{
	//must be at least one letter on the bottom row to be able to move there
	if (_bot[0] == 0) return;
   	_bCursorTop = false;
	_cx=0;
	cursorLast();
}

int Roundels::cursorAt(Point pt)
{
	int i;
	for (i = 0; i < (signed) _top.size(); i++)
	{
		if (_top[i] && _top[i]->_spr->contains(pt))
		{
			_bCursorTop = true;
			_cx = i;
			return i+1;
		}
	}
	for(i = _bot.size()-1; i>=0; i--)
	{
		if (_bot[i])
		{
			if (_bot[i]->_spr->contains(pt))
			{
				_bCursorTop = false;
				_cx = i;
				return i+1;
			}
			break; // only the last letter on the bottom is clickable
		}
	}
	return 0;
}

//move the letters currently on the bottom row back
//to the top, to the gaps already there
void Roundels::clearAllToTop(bool bResetCursor /*=true*/)
{
	if (isMoving()) return;	//can't move if any already moving

	//make sure players cursor is on top line
	_bCursorTop = true;
	if (bResetCursor) cursorFirst();

	int xx = 0;
	//move letters in bottom row to gaps in top
	for (xx=0; xx < (int)_word.length(); ++xx)
	{
		if (_bot[xx] == 0) return; 	//no more in bottom row

		_cx = xx;
		moveLetterUp(false);
	}
}

//redisplay (as selected tiles) the last word that was tried
//Allows player to quickly redo a word and add or remove a letter at the end
void Roundels::setWordToLast()
{
	if (isMoving()) return;	//can't move if any already moving

	int oldcx = _cx;
	clearAllToTop(false);
	int xtop = 0;
	int xlast = 0;
	for (xlast=0; xlast < (int)_word.length(); ++xlast)
	{
		if (0 == _last[xlast]) break;	//no more letters in last[] array
		for (xtop=0; xtop < (int)_word.length(); ++xtop)
		{
			if (_top[xtop] == _last[xlast])
			{
				_cx = xtop;
				moveLetterDown(false);
				break;
			}
		}
	}
	_cx = oldcx;
}

//return the collected letters on the bottom row as a string
//so we can test it against dictionary words and give points
std::string Roundels::getBottomWord()
{
	std::string newword;
	bool bSaveLast = (_bot[0] && _bot[1] && _bot[2]); //at least 3 letter to be tried
	int xx;
	for (xx=0; xx < (int)_word.length(); ++xx)
	{
		//if a valid char then add it to the newword string
		if (_bot[xx] != 0) newword += _bot[xx]->_letter;

		//save curr bottom word to 'last' array so we can put
		//it back if player asks. Only save if letters on bottom exist
		if (bSaveLast) _last[xx] = _bot[xx];
	}
	return newword;
}


