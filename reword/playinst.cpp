////////////////////////////////////////////////////////////////////
/*

File:			playinst.cpp

Class impl:		PlayInst

Description:	A class derived from the IPlay interface to handle all screen
				events and drawing of the Instructions screen

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.5		28.05.08	Added touchscreen support

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
#include "playinst.h"
#include "helpers.h"
#include "platform.h"
#include <sstream>


PlayInst::PlayInst(GameData &gd)  : _gd(gd)
{
	_running = false;
	_init = false;
}

void PlayInst::init(Input *input)
{
	//once the class is initialised, init and running are set true

	_page = 0;
	nextPage();	//start at page 1

	_title.setWordCenterHoriz(std::string("INFO"), _gd._letters, (BG_LINE_TOP-_gd._letters.tileH())/2, 2);
	_title.startMoveFrom( 0, -(_gd._letters.tileH()*2), 15, 100, 0, ROUNDEL_VEL);
	_titleW.start(3000, 1000);
	
	//set the repeat of the keys required
	input->setRepeat(Input::UP, 250, 250);		//button, rate, delay
	input->setRepeat(Input::DOWN, 250, 250);
	
	//set arrow (scroll positions)
	_gd._arrowUp.setPos(SCREEN_WIDTH-_gd._arrowUp.tileW(), BG_LINE_TOP+2);		//positions dont change, just made visible or not if scroll available
	_gd._arrowUp.setFrame(_gd._arrowUp.getMaxFrame()-1);			//last frame
	_gd._arrowUp.setTouchable(true);	//always touchable even if invisible
	_gd._arrowDown.setPos(SCREEN_WIDTH-_gd._arrowDown.tileW(), BG_LINE_BOT-_gd._arrowDown.tileH()-2);
	_gd._arrowDown.setFrame(_gd._arrowDown.getMaxFrame()-1);			//last frame
	_gd._arrowDown.setTouchable(true);	//always touchable even if invisible

	//calc number of lines available for displaying instruction lines
	//end of display area minus start of screen lines+title height, div by line height. Minus 1 for a reasonable gap
	_lines = ((BG_LINE_BOT - BG_LINE_TOP - _gd._fntMed.height()) / _gd._fntClean.height()) - 1;
	
	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

void PlayInst::render(Screen *s)
{
	if (!_init) return;

	_gd._menubg.blitTo( s );

	//draw screen title
	_title.draw(s);

	int yyStart = BG_LINE_TOP + 5;	//gap below roundels to put title
	int helpYpos = BG_LINE_BOT+((SCREEN_HEIGHT-BG_LINE_BOT-_gd._fntClean.height())/2);
	if (_page == 1)
	{
		_gd._fntMed.put_text(s, yyStart, "About", BLUE_COLOUR, true);
		_gd._fntClean.put_text(s, helpYpos, "Press B for controls, Y to exit", GREY_COLOUR, true);
	}
	
	if (_page == 2)
	{
		_gd._fntMed.put_text(s, yyStart, "Controls", BLUE_COLOUR, true);
		_gd._fntClean.put_text(s, helpYpos, "Press B for rules, Y to exit", GREY_COLOUR, true);
	}
	
	if (_page == 3)
	{
		_gd._fntMed.put_text(s, yyStart, "How to play", BLUE_COLOUR, true);
		_gd._fntClean.put_text(s, helpYpos, "Press B for scoring, Y to exit", GREY_COLOUR, true);
	}
	else if (_page == 4)
	{
		_gd._fntMed.put_text(s, yyStart, "Scoring", BLUE_COLOUR, true);
		_gd._fntClean.put_text(s, helpYpos, "Press B or Y to exit", GREY_COLOUR, true);
	}

	//draw the text here... use same code as drawing dictionary...
	int yy = yyStart + _gd._fntMed.height() +
			(((int)_inst.size() > _lines)?0:(((_lines-(int)_inst.size())/2)*_gd._fntClean.height()));
	std::vector<std::string>::const_iterator it = _inst.begin() + _instLine;	//add offset
	int lines = 0;
	while (it != _inst.end())
	{
		if (_bCentered)
			_gd._fntClean.put_text(s, yy, (*it).c_str(), _txtColour, false);
		else
			_gd._fntClean.put_text(s, 20, yy, (*it).c_str(), _txtColour, false);
		lines++;
		yy+=_gd._fntClean.height();
		if (lines >= _lines) break;
		++it;
	}

	_gd._arrowUp.draw(s);		//only if set visible (more lines than screen shows)
	_gd._arrowDown.draw(s);		//..

}

void PlayInst::work(Input *input, float speedFactor)
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

    if (input->repeat(Input::UP))	button(input, Input::UP);
    if (input->repeat(Input::DOWN)) button(input, Input::DOWN);

	_gd._arrowUp.setVisible(_instLine > 0);
	_gd._arrowUp.work();
	_gd._arrowDown.setVisible(_instLine < (int)_inst.size()-_lines);
	_gd._arrowDown.work();
}

void PlayInst::button(Input *input, Input::ButtonType b)
{
	switch (b)
	{
	case Input::UP: 
		if (input->isPressed(b)) 
			//move the _instLine offset var up a line
			scrollDown();
		break;
	case Input::DOWN: 
		if (input->isPressed(b))
			//move the _instLine offset var down a line
			scrollUp();
		break;
	case Input::Y: 
	case Input::START: 
		if (input->isPressed(b))	//exit back to menu
		{
			_gd._state = ST_MENU;
			_running = false;	//exit this class running state
		}
		break;
	case Input::CLICK: 
	case Input::B: 
		if (input->isPressed(b))
			nextPage();
		break;
	default:break;
	}
}

void PlayInst::nextPage()
{
	buildPage(++_page);
	if (_page>4)
	{
		_gd._state = ST_MENU;		//back to menu
		_running = false;
	}
}
void PlayInst::scrollDown()
{
	if ((int)_inst.size()>_lines && _instLine > 0) --_instLine;
}
void PlayInst::scrollUp()
{
	if ((int)_inst.size()>_lines && _instLine < (int)_inst.size()-_lines) ++_instLine;
}

void PlayInst::touch(Point pt)
{
	//check if touch scroll arrows
	if (_gd._arrowUp.contains(pt))
	{
		if (_gd._arrowUp.isTouchable())
			_gd._arrowUp.startAnim(0, -1, ImageAnim::ANI_ONCE, 40);
		scrollDown();
	}
	else if (_gd._arrowDown.contains(pt))
	{
		if (_gd._arrowDown.isTouchable())
			_gd._arrowDown.startAnim(0, -1, ImageAnim::ANI_ONCE, 40);
		scrollUp();
	}
	else
		if (!_doubleClick.done())
			nextPage();
		else
			_doubleClick.start(300);
}

void PlayInst::buildPage(int page)
{
	//now build the scrolling instructions strings to display (on seperate lines)
	_instLine = 0;	//init start of scrollable text, start on first line

	std::stringstream strstr;
	std::string str;
	_bCentered = false;
	switch (page)
	{
	case 1:strstr <<
			"\n\n"
			"Written by : Alistair McLuckie (PurplePup)\n\n"
			"Ideas & testing by : Annette Odom\n\n"
			"Music by : unknown artist\n\n"
			;
			_txtColour = BLACK_COLOUR;
			_bCentered = true;
			break;
	case 2:strstr <<
			"During play:\n\n"
			"B  -  select a letter\n"
			"X  -  try the word against dictionary\n"
			"Y  -  clear the word\n"
			"A  -  jumble the available letters\n"
			"START or P  -  pause game (easy mode only)\n"
			"SELECT -  in-game skip or quit options\n"
			"L or R  -  select last word\n"
			"\nAt end of level:\n\n"
			"B  -  continue to next level\n"
			"Y  -  show dictionary definition\n"
			"\nAdditional options are displayed on screen at the required time."
			;
			_txtColour = BLACK_COLOUR;
			break;
	case 3:	strstr <<	//instructions
			"There are three game modes to play, with three difficulty levels "
			"each. Time allowed and bonuses depend on the difficulty. \n\n"
			"1) Reword\n"
			"Make 3, 4, 5 and 6 letter words from the 6 letters given. "
			"You must make at least one 6 letter word in each round to be able "
			"to move on to the next round. If all words are found, "
			"a bonus is given for each second remaining."
			"\n\n"
			"2) Speed6\n"
			"You must get a single 6 letter word to continue to the next round.\n"
			"A bonus is given each time a fastest word is achieved."
			"\n\n"
			"3) TimeTrial\n"
			"You must get as many 6 letter words as you can in the time allowed.\n"
			"A bonus is given each time a fastest word is achieved.\n"
			"Note that the timer will continue to count down even after a word has been "
			"found. You must move on as quickly as possible for a good score!"
			"\n\n"
			"General Info:\n"
			"In Reword or Speed6 you can press SELECT for 'skip' and 'quit' options."
			;
			_txtColour = BLACK_COLOUR;
			break;
			
	case 4: strstr <<	//scoring
			"In Reword:\n" <<
			SCORE_BONUS <<" for each level passed\n" <<
			SCORE_SECONDS << " * seconds left, if ALL words found\n" <<
			SCORE_WORD6 << " for each 6 letter word found\n" <<
			SCORE_WORD << " for each 3, 4, 5 letter word\n" <<
			"\nIn Speed6:\n" <<
			SCORE_WORD6 << " * for each 6 letter word found\n" <<
			SCORE_FASTEST << " * remainder seconds for each fastest\n" <<
			"\nIn TimeTrial:\n" <<
			SCORE_WORD6 << " for each 6 letter word found\n" <<
			SCORE_FASTEST << " * remainder seconds for each fastest\n"
			;
			_txtColour = BLACK_COLOUR;
			break;
			
	default:return;	//not used
	}
	str = strstr.str();
	pp_s::buildTextPage(str, FONT_CLEAN_MAX, _inst);	//pass max chars wide displayable
}



