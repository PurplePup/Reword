////////////////////////////////////////////////////////////////////
/*

File:			playgamedict.cpp

Class impl:		PlayGameDict

Description:	A class derived from the IPlay interface to handle the in-game dictionary
				screen after a re-word is found. The dict screen can scroll longer
				descriptions.

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			11 Oct 2011

History:		Version	Date		Change
				-------	----------	--------------------------------

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
#include "playgamedict.h"
#include "utils.h"
#include "helpers.h"
#include "audio.h"
#include "platform.h"

enum { CTRLGRP_SCROLL = 1, CTRLGRP_BUTTONS = 2 };

PlayGameDict::PlayGameDict(GameData& gd, const std::string &strDictWord) :
    _gd(gd),
    _dictWord(strDictWord)
{
}

PlayGameDict::~PlayGameDict()
{
    //dtor
}

void PlayGameDict::init(Input *input)
{
	//set the repeat of the keys required
	input->setRepeat(ppkey::UP, 250, 250);		//button, rate, delay
	input->setRepeat(ppkey::DOWN, 250, 250);
	input->setRepeat(ppkey::LEFT, 250, 250);
	input->setRepeat(ppkey::RIGHT, 250, 250);

    //set arrow (scroll positions)
//    _gd._arrowUp.setPos(SCREEN_WIDTH-_gd._arrowUp.tileW(), BG_LINE_TOP+2);		//positions dont change, just made visible or not if scroll available
//    _gd._arrowUp.setFrameLast();			//last (white) frame
//    _gd._arrowUp.setTouchable(true);	//always touchable even if invisible
//    _gd._arrowDown.setPos(SCREEN_WIDTH-_gd._arrowDown.tileW(), BG_LINE_BOT-_gd._arrowDown.tileH()-2);
//    _gd._arrowDown.setFrameLast();
//    _gd._arrowDown.setTouchable(true);

    {
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_scroll_up.png")));
    p->setPos(SCREEN_WIDTH-p->tileW(), BG_LINE_TOP+2);
    p->_sigEvent2.Connect(this, &PlayGameDict::ControlEvent);
    Control c(p, CTRLID_SCROLL_UP, CTRLGRP_SCROLL, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsDict.add(c);
    }
    {
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_scroll_down.png")));
    p->setPos(SCREEN_WIDTH-p->tileW(), BG_LINE_TOP+2+p->tileH()+6);
    p->_sigEvent2.Connect(this, &PlayGameDict::ControlEvent);
    Control c(p, CTRLID_SCROLL_DOWN, CTRLGRP_SCROLL, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsDict.add(c);
    }

    //get the dictionary definition into one long string
	std::string dictDefinition, dictDefLine;
    dictDefinition = _gd._words.getDictForWord(_dictWord)._description;
    pptxt::trimLeft(dictDefinition, " \t\n\r");	//NOTE need \n\r on GP2X
    pptxt::trimRight(dictDefinition, " \t\n\r"); //ditto
    if (!dictDefinition.length()) dictDefinition = "** sorry, not defined **";	//blank word - no dictionary entry
    //now build definition strings to display (on seperate lines)
    _dictLine = 0;	//start on first line
    pptxt::buildTextPage(dictDefinition, FONT_CLEAN_MAX, _dictDef);

    //prepare roundel class ready for a dictionary display
    tSharedImage &letters = Resource::image("roundel_letters.png");
    _roundDict= tAutoRoundels(new Roundels());
    _roundDict->setWordCenterHoriz(_dictWord, letters, (BG_LINE_TOP-letters.get()->height())/2, 4);
    _roundDict->easeMoveFrom(Screen::width(), 0, 1400, 20);

    _menubg = Resource::image("menubg.png");

    //[BACK] dictionary screen buttons - only shown in dict display
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_back_small.png")));
    p->setPos(8, BG_LINE_BOT + ((SCREEN_HEIGHT - BG_LINE_BOT - p->tileH())/2));
    Control c(p, CTRLID_BACK, CTRLGRP_BUTTONS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsDict.add(c);

	//calc number of lines available for displaying dictionary lines
	//end of display area minus start of screen lines+title height, div by line height. Minus 1 for a reasonable gap
	_lines = ((BG_LINE_BOT - BG_LINE_TOP - _gd._fntMed.height()) / _gd._fntClean.height()) - 1;

    _helpMsg = "Press " + Locator::input().keyDescription(ppkey::Y) + " to go back";

    updateScrollButtons();

	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

void PlayGameDict::render(Screen* s)
{
	//_gd._menubg.blitTo( s );
	//ppg::blit_surface(_gd._menubg.surface(), NULL, s->surface(), 0, 0);
	ppg::blit_surface(_menubg->surface(), NULL, s->surface(), 0, 0);

	_roundDict->render(s);

	_gd._fntClean.put_text(s, BG_LINE_TOP, "Definition:", GREY_COLOUR, true);

	//draw the dictionary text here... previously split into vector string "lines"
	int yy = BG_LINE_TOP + (_gd._fntClean.height()*2) +
			(((int)_dictDef.size() > _lines)?0:(((_lines-(int)_dictDef.size())/2)*_gd._fntClean.height()));
	std::vector<std::string>::const_iterator it = _dictDef.begin() + _dictLine;	//add offset
	int lines = 0;
	while (it != _dictDef.end())
	{
		_gd._fntClean.put_text(s, yy, (*it).c_str(), BLACK_COLOUR, false);
		lines++;
		yy+=_gd._fntClean.height();
		if (lines >= _lines) break;
		++it;
	}

	int helpYpos = BG_LINE_BOT+((SCREEN_HEIGHT-BG_LINE_BOT-_gd._fntClean.height())/2);
	_gd._fntClean.put_text(s, helpYpos, _helpMsg.c_str(), GREY_COLOUR, true);

	_controlsDict.render(s);
}

void PlayGameDict::work(Input* input, float speedFactor)
{
	//Do repeat keys...
	//if a key is pressed and the interval has expired process
	//that button as if pressesd again

    if (input->repeat(ppkey::UP))	button(input, ppkey::UP);
    if (input->repeat(ppkey::DOWN)) button(input, ppkey::DOWN);
    if (input->repeat(ppkey::LEFT))	button(input, ppkey::LEFT);
    if (input->repeat(ppkey::RIGHT)) button(input, ppkey::RIGHT);

	_roundDict->work(input, speedFactor);
    _controlsDict.work(input, speedFactor);
}

bool PlayGameDict::button(Input* input, ppkey::eButtonType b)
{
	switch (b)
	{
	case ppkey::UP:
		if (input->isPressed(b))
			//move the _dictLine offset var up a line
			scrollDictDown();
		break;
	case ppkey::DOWN:
		if (input->isPressed(b))
			//move the _dictLine offset var down a line
			scrollDictUp();
		break;
	case ppkey::Y:
	case ppkey::CLICK:
		if (input->isPressed(b))
            ppg::pushSDL_Event(USER_EV_EXIT_SUB_SCREEN);
		break;

	default:return false;
	}

    updateScrollButtons();
	return true;
}
void PlayGameDict::scrollDictUp()
{
	if ((int)_dictDef.size()>_lines && _dictLine < (int)_dictDef.size()-_lines) ++_dictLine;
}
void PlayGameDict::scrollDictDown()
{
	if ((int)_dictDef.size()>_lines && _dictLine > 0) --_dictLine;
}

bool PlayGameDict::touch(const Point &pt)
{
    _controlsDict.touched(pt);    //needed to highlight a touched control

//    //tap background to exit dict screen
//		if (!_doubleClick.done())
//			ppg::pushSDL_Event(USER_EV_EXIT_SUB_SCREEN);
//		else
//			_doubleClick.start(300);

    return true;
}

//releasing 'touch' press
bool PlayGameDict::tap(const Point &pt)
{
    const int ctrl_id = _controlsDict.tapped(pt);

    if (ctrl_id == CTRLID_BACK)
    {
        ppg::pushSDL_Event(USER_EV_EXIT_SUB_SCREEN);
        return true;
    }
    else if (ctrl_id == CTRLID_SCROLL_UP)
    {
		scrollDictDown();
    }
    else if (ctrl_id == CTRLID_SCROLL_DOWN)
    {
		scrollDictUp();
    }


    return false;
}

void PlayGameDict::updateScrollButtons()
{
    const bool bShow = ((int)_dictDef.size()>_lines);
    _controlsDict.showGroup(bShow, CTRLGRP_SCROLL);
    if (bShow)
    {
        _controlsDict.enableControl((_dictLine > 0), CTRLID_SCROLL_UP);
        _controlsDict.enableControl((_dictLine < (int)_dictDef.size()-_lines), CTRLID_SCROLL_DOWN);
    }
}

//event signal from imageanim indicating end of animation
void PlayGameDict::ControlEvent(int event, int ctrl_id)
{
    (void)(ctrl_id);    //unused

    if (event == USER_EV_END_ANIMATION)
    {
        updateScrollButtons();
    }
}





