////////////////////////////////////////////////////////////////////
/*

File:			playhigh.cpp

Class impl:		PlayHigh

Description:	A class derived from the IPlay interface to handle all screen
				events and drawing of the Hi-Score screen

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4		06.03.2008	Added speed6 and timetrial displays and mode logos
				0.5		28.05.08	Added touchscreen support
				0.5.1	08.12.08	Pandora screen size support

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
#include "playhigh.h"
#include "platform.h"
#include <cassert>
#include <memory>

PlayHigh::PlayHigh(GameData &gd)  : _gd(gd)
{
	_running = false;
	_init = false;
}

void PlayHigh::init(Input *input)
{
	//once the class is initialised, init and running are set true

	//play menu music - if not already playing - as we could come
	//here from the end of the game rather than from the menu
	if (!Mix_PlayingMusic())
		Mix_PlayMusic(_gd._musicMenu, -1);	//play 'forever'

	//set up local difficulty settings
	setDifficulty(_gd._diffLevel);
	setMode(_gd._mode);	//calls prepareBackground();

	_currPos = 0;
	//set pos to -1 if not editing a high score, ie -1=editing high score
	_pos = (ST_HIGHEDIT==_gd._state)?_gd._score.isHiScore(_mode, _diff):-1;

	_curr = _gd._score.curr();

	_title.setWordCenterHoriz(std::string("HISCORE"), _gd._letters, (BG_LINE_TOP-_gd._letters.tileH())/2, 2);
	_title.startMoveFrom( 0, -(_gd._letters.tileH()*2), 15, 100, 0, ROUNDEL_VEL);
	_titleW.start(3000, 1000);

	//set the repeat of the keys required
	input->setRepeat(pp_i::UP, 100, 300);		//button, rate, delay
	input->setRepeat(pp_i::DOWN, 100, 300);
	input->setRepeat(pp_i::LEFT, 100, 300);
	input->setRepeat(pp_i::RIGHT, 100, 300);

	//set arrow (scroll positions)
	_gd._arrowUp.setPos(SCREEN_WIDTH-_gd._arrowUp.tileW(), BG_LINE_TOP+2);		//positions dont change, just made visible or not if scroll available
	_gd._arrowUp.setFrame(_gd._arrowUp.getMaxFrame()-1);			//last frame
	_gd._arrowUp.setTouchable(true);	//always touchable even if invisible
	_gd._arrowDown.setPos(SCREEN_WIDTH-_gd._arrowDown.tileW(), BG_LINE_BOT-_gd._arrowDown.tileH()-2);
	_gd._arrowDown.setFrame(_gd._arrowDown.getMaxFrame());			//last frame
	_gd._arrowDown.setTouchable(true);	//always touchable even if invisible
	_gd._arrowLeft.setPos(2, BG_LINE_BOT-_gd._arrowDown.tileH()-2);		//positions dont change, just made visible or not if scroll available
	_gd._arrowLeft.setFrame(_gd._arrowLeft.getMaxFrame());			//last) frame
	_gd._arrowLeft.setTouchable(true);	//always touchable even if invisible
	_gd._arrowRight.setPos(SCREEN_WIDTH-(_gd._arrowDown.tileW()*2), BG_LINE_BOT-_gd._arrowDown.tileH()-2);
	_gd._arrowRight.setFrame(_gd._arrowRight.getMaxFrame());			//last frame
	_gd._arrowRight.setTouchable(true);	//always touchable even if invisible

	//calc hiscore table element positions

	int yUsed = (BG_LINE_BOT - BG_LINE_TOP) - (int)(_gd._fntClean.height() * 10.5);
	_yyGap = yUsed / 11;

	int itemGaps = 5;	//between WWW 00000000 0000w 000s
	_xInitsLen = _gd._fntClean.calc_text_length("WWW");
	_xScoreLen = _gd._fntClean.calc_text_length("00000000");
	_xWordsLen = _gd._fntClean.calc_text_length("0000w");
	_xTimesLen = _gd._fntClean.calc_text_length("000s");
	int xUsed = _xInitsLen + _xScoreLen + _xWordsLen + _xTimesLen;
	_xDiffLen = _gd._fntClean.calc_text_length("MEDIUM");	//longest of EASY/MEDIUM/HARD
	_xCharLen = _gd._fntClean.calc_text_length("W");
	int maxGap = _gd._fntClean.calc_text_length("XXXXX");
	_xxGap = (Screen::width() - (_xDiffLen + xUsed)) / itemGaps;
	if (_xxGap > maxGap) _xxGap = maxGap;	//for smaller screens (GP2X)
	_xxStart = _xDiffLen + _xxGap;

	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

void PlayHigh::render(Screen *s)
{
	assert(_init);

	//display:

	//           HIGHSCORES :
	//
	//   ABC ...... 2000000 (#words)
	//   GFG ...... 1232322 (#words)
	//   TTT ......  109999 (#words)
	//   AAA ......    4344 (#words)
	//
	//

	_menubg->blitTo( s );

	//draw screen title
	_title.draw(s);

	char sLetter[2] = {0x00, 0x00};	//"string" to hold editable letters

	int line = 0;			//'curr' line
	int yyLine = 0;			//count lines so far
	int yy = BG_LINE_TOP + _yyGap;	//actual line y pix pos

	_gd._fntClean.put_text(s, (_xxStart - _xDiffLen)/2, yy, _description.c_str(), _diffColour, false);

	int xx(0);
	while (yyLine < 10)
	{
		xx = _xxStart;
		if (yyLine == _pos)	//if it's -1 (not editing) then this wont display
		{
			//draw current score in the gap for player to enter inits
			sLetter[0] = _curr.inits[0];
			_gd._fntClean.put_text(s, xx, yy, sLetter, (0==_currPos)?BLACK_COLOUR:_diffColour);
			sLetter[0] = _curr.inits[1];
			_gd._fntClean.put_text(s, xx+_xCharLen, yy, sLetter, (1==_currPos)?BLACK_COLOUR:_diffColour);
			sLetter[0] = _curr.inits[2];
			_gd._fntClean.put_text(s, xx+_xCharLen+_xCharLen, yy, sLetter, (2==_currPos)?BLACK_COLOUR:_diffColour);
			xx += _xInitsLen + _xxGap;

			_gd._fntClean.put_number(s, xx, yy, _curr.score, "%08d", _diffColour);
			xx += _xScoreLen + _xxGap;
			_gd._fntClean.put_number(s, xx, yy, _curr.words, "%04dw", _diffColour);
			xx += _xWordsLen + _xxGap;
			if (_mode > GM_REWORD)  //speed or timetrial show speed
				_gd._fntClean.put_number(s, xx, yy, _curr.fastest, "%03ds", _diffColour);
		}
		else
		{
			//draw scores in order as normal
			_gd._fntClean.put_text(s, xx, yy, _gd._score.inits(_mode, _diff, line).c_str(), _diffColour);
			xx += _xInitsLen + _xxGap;
			_gd._fntClean.put_number(s, xx, yy, _gd._score.score(_mode, _diff, line), "%08d", BLUE_COLOUR);
			xx += _xScoreLen + _xxGap;
			_gd._fntClean.put_number(s, xx, yy, _gd._score.words(_mode, _diff, line), "%04dw", BLUE_COLOUR);
			xx += _xWordsLen + _xxGap;
			if (_mode > GM_REWORD)  //speed or timetrial show speed
				_gd._fntClean.put_number(s, xx, yy, _gd._score.fastest(_mode, _diff, line), "%03ds", BLUE_COLOUR);

			++line;	//dont jump a score (ie line = 0->9, but yyLine will jump+1 if yyLine==pos
		}
		++yyLine;
		yy += _gd._fntClean.height() + _yyGap;
	}

	int helpYpos = BG_LINE_BOT+((SCREEN_HEIGHT-BG_LINE_BOT-_gd._fntClean.height())/2);
	if (!isEditing())
		_gd._fntClean.put_text(s, helpYpos, "Up/down:mode, left/right:difficulty, B to exit", GREY_COLOUR, true);
	else
		_gd._fntClean.put_text(s, helpYpos, "Enter initials then B to save", GREY_COLOUR, true);

	_gd._arrowUp.draw(s);		//only if set visible (more lines than screen shows)
	_gd._arrowDown.draw(s);
	_gd._arrowLeft.draw(s);
	_gd._arrowRight.draw(s);

}

void PlayHigh::work(Input *input, float speedFactor)
{
	_title.work();

	//animate the roundel title if it's not moving and
	//we have waited long enough since it animated last
	if (!_title.isMoving() && _titleW.done(true))
	{
		if (_title.isInOrder())
			_title.jumbleWord(true);
		else
			_title.unJumbleWord(true);
	}

	//Do repeat keys...
	//if a key is pressed and the interval has expired process
	//that button as if pressesd again

    if (input->repeat(pp_i::UP))	button(input, pp_i::UP);
    if (input->repeat(pp_i::DOWN)) button(input, pp_i::DOWN);
    if (input->repeat(pp_i::LEFT))	button(input, pp_i::LEFT);
    if (input->repeat(pp_i::RIGHT)) button(input, pp_i::RIGHT);

	//_pos -1 = in edit hiscore initials mode
	_gd._arrowUp.setVisible(_mode < GM_MAX-1 && !isEditing());
	_gd._arrowUp.work();
	_gd._arrowDown.setVisible(_mode > GM_ARCADE && !isEditing());
	_gd._arrowDown.work();
	_gd._arrowLeft.setVisible(_diff > DIF_EASY && !isEditing());
	_gd._arrowLeft.work();
	_gd._arrowRight.setVisible(_diff < DIF_MAX-1 && !isEditing());
	_gd._arrowRight.work();
}

void PlayHigh::button(Input *input, pp_i::eButtonType b)
{
	switch (b)
	{
	case pp_i::UP:
		if (input->isPressed(b))
			moveUp();
		break;
	case pp_i::DOWN:
		if (input->isPressed(b))
			moveDown();
		break;
	case pp_i::LEFT:
		if (input->isPressed(b))
			moveLeft();
		break;
	case pp_i::RIGHT:
		if (input->isPressed(b))
			moveRight();
		break;
	case pp_i::CLICK:
	case pp_i::B:
		if (input->isPressed(b))
		{
			if (isEditing() && _currPos < 2)
			{
				_currPos++;	//treat as button right unless on 3rd char
				break;
			}
			if (isEditing())	//is editing and position must be 2 (3rd char)
			{
				//so player has pressed B on last char to save to entered inits
				_gd._score.insert(_mode, _diff, _pos, _curr);
				_gd._score.save();	//save now so player can switch off or return to menu if wishes

				_pos = -1;		//set to not-editing
				break;
			}
			//else pos is -1 (not editing) so follow on to exit...
		}
		//follow on to exit to ST_MENU ...
		//not break
	case pp_i::X:
		if (input->isPressed(b))
		{
			_gd._state = ST_MENU;
			_running = false;	//exit this class running state
		}
		break;
	default:break;
	}
}

void PlayHigh::moveUp()
{
	//move from C to B to A
	if (isEditing())
	{
		if (_curr.inits[_currPos] > ' ') --_curr.inits[_currPos];
	}
	else
		setMode((eGameMode)--_mode);
}

void PlayHigh::moveDown()
{
	//move from A to B to C
	if (isEditing())
	{
		if (_curr.inits[_currPos] < 'Z') ++_curr.inits[_currPos];
	}
	else
		setMode((eGameMode)++_mode);
}

void PlayHigh::moveLeft()
{
	if (isEditing())
	{
		if (_currPos > 0) _currPos--;
	}
	else
		setDifficulty((eGameDiff)(_diff-1));
}

void PlayHigh::moveRight()
{
	if (isEditing())
	{
		if (_currPos < 2) _currPos++;
	}
	else
		setDifficulty((eGameDiff)(_diff+1));
}

bool PlayHigh::touch(const Point &pt)
{
	//check if touch scroll arrows
	if (_gd._arrowUp.contains(pt))
	{
		if (_gd._arrowUp.isTouchable())
			_gd._arrowUp.startAnim(0, -1, ImageAnim::ANI_ONCE, 40);
		moveDown();
        return true;
	}
	else if (_gd._arrowDown.contains(pt))
	{
		if (_gd._arrowDown.isTouchable())
			_gd._arrowDown.startAnim(0, -1, ImageAnim::ANI_ONCE, 40);
		moveUp();
        return true;
	}
	else if (_gd._arrowLeft.contains(pt))
	{
		if (_gd._arrowLeft.isTouchable())
			_gd._arrowLeft.startAnim(0, -1, ImageAnim::ANI_ONCE, 40);
		moveLeft();
        return true;
	}
	else if (_gd._arrowRight.contains(pt))
	{
		if (_gd._arrowRight.isTouchable())
			_gd._arrowRight.startAnim(0, -1, ImageAnim::ANI_ONCE, 40);
		moveRight();
        return true;
	}
	else
	{
		if (!_doubleClick.done())
		{
			_gd._state = ST_MENU;
			_running = false;	//exit this class running state
		}
		else
			_doubleClick.start(300);
        return true;
    }
    return false;

}

//set difficulty locally
void PlayHigh::setDifficulty(eGameDiff diff)
{
	if (diff < DIF_EASY) diff = DIF_EASY;
	if (diff > DIF_HARD) diff = DIF_HARD;
	_diff = diff;
	_diffColour = (DIF_EASY==_diff)?GREEN_COLOUR:(DIF_MED==_diff)?ORANGE_COLOUR:RED_COLOUR;
	setDescription();
}
//set mode locally
void PlayHigh::setMode(eGameMode mode)
{
	if (mode < GM_ARCADE) mode = GM_ARCADE;
	if (mode >= GM_MAX) mode = (eGameMode)((int)GM_MAX - 1);
	_mode = mode;
	setDescription();
	prepareBackground();
}

void PlayHigh::setDescription()
{
	switch (_diff)
	{
	case DIF_EASY:		_description = "EASY";break;
	case DIF_MED:		_description = "MEDIUM";break;
	case DIF_HARD:		_description = "HARD";break;
	default:break;
	}
}

void PlayHigh::prepareBackground()
{
	//create the background to be used for this level,
	//pre drawing so we dont need to do it each frame.
	//...
	_menubg = std::auto_ptr<Image>(new Image());
	_menubg->createThisFromImage(_gd._menubg_plain);	//copy of basic menubg without roundel

	int x = - (_gd._menu_reword.tileW() /6);	//slightly off screen
	int y = ((SCREEN_HEIGHT - _gd._menu_reword.tileH()) / 2) + 2;	//center (+2 for gp2x too high)

	switch (_mode)
	{
	case GM_ARCADE:		_menubg->blitFrom(&_gd._menu_arcade, -1, x, y); //##TODO
						break;
	case GM_REWORD:		_menubg->blitFrom(&_gd._menu_reword, -1, x, y);
						break;
	case GM_SPEED6:		_menubg->blitFrom(&_gd._menu_speed6, -1, x, y);
						break;
	case GM_TIMETRIAL:	_menubg->blitFrom(&_gd._menu_timetrial, -1, x, y);
						break;
	default:break;
	}
}

