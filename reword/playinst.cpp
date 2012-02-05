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
#include <boost/bind.hpp>

enum { CTRLGRP_SCROLL = 1,CTRLGRP_BUTTONS = 2 };

PlayInst::PlayInst(GameData &gd)  : _gd(gd)
{
	_running = false;
	_init = false;
}

void PlayInst::init(Input *input)
{
	//once the class is initialised, init and running are set true

	_title.setWordCenterHoriz(std::string("INFO"), _gd._letters, (BG_LINE_TOP-_gd._letters.tileH())/2, 2);
	_title.startMoveFrom( 0, -(_gd._letters.tileH()*2), 15, 100, 0, ROUNDEL_VEL);
	_titleW.start(3000, 1000);

	//set the repeat of the keys required
	input->setRepeat(ppkey::UP, 250, 250);		//button, rate, delay
	input->setRepeat(ppkey::DOWN, 250, 250);

	//calc number of lines available for displaying instruction lines
	//end of display area minus start of screen lines+title height, div by line height. Minus 1 for a reasonable gap
	_lines = ((BG_LINE_BOT - BG_LINE_TOP - _gd._fntMed.height()) / _gd._fntClean.height()) - 1;

	//set arrow controls (scroll positions)
    {
    boost::shared_ptr<Sprite> p(new Sprite(RES_BASE + "images/btn_round_scroll_up.png", 0, 5));
    p->setPos(SCREEN_WIDTH-p->tileW(), BG_LINE_TOP+2);
    p->_sigEvent.connect(boost::bind(&PlayInst::ControlEvent, this, _1, _2));
    Control c(p, CTRLID_SCROLL_UP, CTRLGRP_SCROLL, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsInst.add(c);
    }
    {
    boost::shared_ptr<Sprite> p(new Sprite(RES_BASE + "images/btn_round_scroll_down.png", 0, 5));
    p->setPos(SCREEN_WIDTH-p->tileW(), BG_LINE_BOT-p->tileH()-2);
    p->_sigEvent.connect(boost::bind(&PlayInst::ControlEvent, this, _1, _2));
    Control c(p, CTRLID_SCROLL_DOWN, CTRLGRP_SCROLL, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsInst.add(c);
    }

    //load EXIT/NEXT buttons
    {
    boost::shared_ptr<Sprite> p(new Sprite(RES_BASE + "images/btn_square_exit_small.png", 255, 5));
    p->setPos(8, BG_LINE_BOT + ((SCREEN_HEIGHT - BG_LINE_BOT - p->tileH())/2));
    Control c(p, CTRLID_EXIT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsInst.add(c);
    }
    {
    boost::shared_ptr<Sprite> p(new Sprite(RES_BASE + "images/btn_square_next_small.png", 255, 5));
    p->setPos(SCREEN_WIDTH - p->tileW() - 8, BG_LINE_BOT + ((SCREEN_HEIGHT - BG_LINE_BOT - p->tileH())/2));
    Control c(p, CTRLID_NEXT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsInst.add(c);
    }

	_page = 0;
	nextPage();	//start at page 1 - updates scroll buttons

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

    _controlsInst.render(s);
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

    if (input->repeat(ppkey::UP))	button(input, ppkey::UP);
    if (input->repeat(ppkey::DOWN)) button(input, ppkey::DOWN);

    _controlsInst.work(input, speedFactor);
}

void PlayInst::updateScrollButtons()
{
    const bool bShow = ((int)_inst.size()>_lines && _page > 0);
    _controlsInst.showGroup(bShow, CTRLGRP_SCROLL);
    if (bShow)
    {
        _controlsInst.enableControl((_instLine > 0), CTRLID_SCROLL_UP);
        _controlsInst.enableControl((_instLine < (int)_inst.size()-_lines), CTRLID_SCROLL_DOWN);
    }
}

//event signal from imageanim indicating end of animation
void PlayInst::ControlEvent(int event, int control_id)
{
    (void)(control_id);

    if (event == USER_EV_END_ANIMATION)
    {
        updateScrollButtons();
    }
}


void PlayInst::button(Input *input, ppkey::eButtonType b)
{
	switch (b)
	{
	case ppkey::UP:
		if (input->isPressed(b))
			//move the _instLine offset var up a line
			scrollDown();
		break;
	case ppkey::DOWN:
		if (input->isPressed(b))
			//move the _instLine offset var down a line
			scrollUp();
		break;
	case ppkey::Y:
	case ppkey::START:
		if (input->isPressed(b))	//exit back to menu
		{
			_gd._state = ST_MENU;
			_running = false;	//exit this class running state
		}
		break;
	case ppkey::CLICK:
	case ppkey::B:
		if (input->isPressed(b))
			nextPage();
		break;
	default:break;
	}

    updateScrollButtons();
}

void PlayInst::nextPage()
{
	buildPage(++_page);
	if (_page>4)
	{
		_gd._state = ST_MENU;		//back to menu
		_running = false;
	}
    updateScrollButtons();
}
void PlayInst::scrollDown()
{
	if ((int)_inst.size()>_lines && _instLine > 0) --_instLine;
}
void PlayInst::scrollUp()
{
	if ((int)_inst.size()>_lines && _instLine < (int)_inst.size()-_lines) ++_instLine;
}

bool PlayInst::touch(const Point &pt)
{
    const int crtl_id = _controlsInst.touched(pt);    //needed to highlight a touched control

	//check if touch scroll arrows
	if (crtl_id == CTRLID_SCROLL_UP)
	{
		scrollDown();
		return true;
	}
	else if (crtl_id == CTRLID_SCROLL_DOWN)
	{
		scrollUp();
		return true;
	}
//	else
//	{
//		if (!_doubleClick.done())
//			nextPage();
//		else
//			_doubleClick.start(300);
//        return true;
//	}
	return false;
}

//releasing 'touch' press
bool PlayInst::tap(const Point &pt)
{
    const int crtl_id = _controlsInst.tapped(pt);

    if (crtl_id == CTRLID_NEXT)
    {
        nextPage();
        return true;
    }
    if (crtl_id == CTRLID_EXIT)
    {
		_gd._state = ST_MENU;		//back to menu
		_running = false;
        return true;
    }

    return false;
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
			"Title music by : unknown MOD artist\n\n"
			;
			_txtColour = BLACK_COLOUR;
			_bCentered = true;
			break;
	case 2:strstr <<
            "Touch screen options:\n\n"
            "Tap a menu option to select, or tap and hold to display "
            "helpful messages. Slide away while holding to cancel "
            "the selection.\n"
            "Press buttons on screen where given, or anywhere else for "
            "the default action."
            "\n\n"
			"Controls during play:\n\n"
			"B  -  select a letter\n"
			"X  -  try the word against dictionary\n"
			"Y  -  clear the word\n"
			"A  -  jumble the available letters\n"
			"START or P  -  pause game (easy mode only)\n"
			"SELECT -  in-game skip or quit options\n"
			"L or R  -  select last word\n"
			"\nAt end of level:\n\n"
			"B  -  continue to next level\n"
			"Y  -  show dictionary definition of highlighted words\n"
			"\nAdditional options are displayed on screen as required."
			;
			_txtColour = BLACK_COLOUR;
			break;
	case 3:	strstr <<	//instructions
			"There are four game modes to play, with three difficulty levels "
			"each. Time allowed and bonuses depend on the difficulty. "
			"You must find at least one of the maximum length words (a Re-word) "
			"to progress or get to the next level in each mode. Other, shorter words "
			"found are a bonus. Pausing causes time penalty, and not available "
			"on hardest difficulty.\n\n"
			"1) Arcade\n"
			"A quick game, allowing you to progress even if you can't get the "
			"all-letter word, as long as you get enough other length words shown "
			"by the highlighted boxes. Longer words for quicker progress."
			"\n\n"
			"2) Reword (Original)\n"
			"Make as many words from the letters given, aiming to use all letters. "
			"You must make at least one word with all letters (a Re-word) in each "
			"round to move on to the next round. If all words are found, "
			"a bonus is given for each second remaining."
			"\n\n"
			"3) SpeedWord\n"
			"You must get a single all-letter (Re-word) word to continue to the "
			"next round.\nA bonus is given each time a fastest word is achieved."
			"\n\n"
			"4) TimeTrial\n"
			"You must get as many all-letter (Re-word) words as you can in the time "
			"allowed.\nA bonus is given each time a fastest word is achieved.\n"
			"Note that the timer will continue to count down even after a word has been "
			"found. You must move on as quickly as possible for a good score!"
			"\n\n"
			"General Info:\n"
			"Press SELECT or touch Menu button in-game to access music tracks, "
			"quick advance, save and exit options.\n"
			;
			_txtColour = BLACK_COLOUR;
			break;

	case 4: strstr <<	//scoring
            "In Arcade:\n" <<
            SCORE_ARCADE << " for enough words to get next level\n" <<
			SCORE_SECONDS << " * seconds left, if ALL words found\n" <<
			SCORE_WORD6 << " for each all-letter word found\n" <<
			SCORE_WORD << " for each smaller letter words\n" <<
			"\nIn Reword:\n" <<
			SCORE_BONUS <<" for each level passed\n" <<
			SCORE_SECONDS << " * seconds left, if ALL words found\n" <<
			SCORE_WORD6 << " for each all-letter word found\n" <<
			SCORE_WORD << " for each smaller letter words\n" <<
			"\nIn SpeedWord:\n" <<
			SCORE_WORD6 << " * for each all-letter word found\n" <<
			SCORE_FASTEST << " * remainder seconds for each fastest\n" <<
			"\nIn TimeTrial:\n" <<
			SCORE_WORD6 << " for each all-letter word found\n" <<
			SCORE_FASTEST << " * remainder seconds for each fastest\n"
			;
			_txtColour = BLACK_COLOUR;
			break;

	default:return;	//not used
	}
	str = strstr.str();
	pptxt::buildTextPage(str, FONT_CLEAN_MAX, _inst);	//pass max chars wide displayable
}



