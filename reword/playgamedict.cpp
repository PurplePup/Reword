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
	input->setRepeat(Input::UP, 250, 250);		//button, rate, delay
	input->setRepeat(Input::DOWN, 250, 250);
	input->setRepeat(Input::LEFT, 250, 250);
	input->setRepeat(Input::RIGHT, 250, 250);

    //set arrow (scroll positions)
    _gd._arrowUp.setPos(SCREEN_WIDTH-_gd._arrowUp.tileW(), BG_LINE_TOP+2);		//positions dont change, just made visible or not if scroll available
    _gd._arrowUp.setFrame(_gd._arrowUp.getMaxFrame());			//last (white) frame
    _gd._arrowUp.setTouchable(true);	//always touchable even if invisible
    _gd._arrowDown.setPos(SCREEN_WIDTH-_gd._arrowDown.tileW(), BG_LINE_BOT-_gd._arrowDown.tileH()-2);
    _gd._arrowDown.setFrame(_gd._arrowDown.getMaxFrame());
    _gd._arrowDown.setTouchable(true);

    //get the dictionary definition into one long string
	std::string dictDefinition, dictDefLine;
    dictDefinition = _gd._words.getDictForWord(_dictWord)._description;
    pp_s::trimLeft(dictDefinition, " \t\n\r");	//NOTE need \n\r on GP2X
    pp_s::trimRight(dictDefinition, " \t\n\r"); //ditto
    if (!dictDefinition.length()) dictDefinition = "** sorry, not defined **";	//blank word - no dictionary entry
    //now build definition strings to display (on seperate lines)
    _dictLine = 0;	//start on first line
    pp_s::buildTextPage(dictDefinition, FONT_CLEAN_MAX, _dictDef);

    //prepare roundel class ready for a dictionary display
    _roundDict= std::auto_ptr<Roundels>(new Roundels());
    _roundDict->setWordCenterHoriz(_dictWord, _gd._letters, (BG_LINE_TOP-_gd._letters.tileH())/2, 4);
    _roundDict->startMoveFrom(Screen::width(), 0, 10, 50, 18, 0);



    //[PREV] dictionary screen buttons - only shown in dict display
    boost::shared_ptr<Sprite> pPrev(new Sprite(RES_BASE + "images/touch_prev.png", 255, 4));
    pPrev->setPos(3, 0);
    pPrev->setTileSize(106, 52, Image::TILE_VERT);
    pPrev->setFrameLast();  //unselected
//    pPrev->setVisible(false);  //not available until countdown finished
    Control cPrev(pPrev, CTRLID_PREV);
    _controlsDict.add(cPrev);

	//calc number of lines available for displaying dictionary lines
	//end of display area minus start of screen lines+title height, div by line height. Minus 1 for a reasonable gap
	_lines = ((BG_LINE_BOT - BG_LINE_TOP - _gd._fntMed.height()) / _gd._fntClean.height()) - 1;

	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

void PlayGameDict::render(Screen* s)
{
	_gd._menubg.blitTo( s );

	_roundDict->draw(s);

	_gd._fntClean.put_text(s, BG_LINE_TOP, "Definition:", GREY_COLOUR, true);

	//draw the dictionary text here... previously split into vector string "lines"
	int yy = BG_LINE_TOP + _gd._fntClean.height() +
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

	_gd._arrowUp.draw(s);		//only if set visible (more lines than screen shows)
	_gd._arrowDown.draw(s);		//..

	int helpYpos = BG_LINE_BOT+((SCREEN_HEIGHT-BG_LINE_BOT-_gd._fntClean.height())/2);
	_gd._fntClean.put_text(s, helpYpos, "Press Y or CLICK to continue", PURPLE_COLOUR, true);

	_controlsDict.render(s);
}

void PlayGameDict::work(Input* input, float speedFactor)
{
	//Do repeat keys...
	//if a key is pressed and the interval has expired process
	//that button as if pressesd again

    if (input->repeat(Input::UP))	button(input, Input::UP);
    if (input->repeat(Input::DOWN)) button(input, Input::DOWN);
    if (input->repeat(Input::LEFT))	button(input, Input::LEFT);
    if (input->repeat(Input::RIGHT)) button(input, Input::RIGHT);

	_roundDict->work();

	_gd._arrowUp.setVisible(_dictLine > 0);
	_gd._arrowUp.work();
	_gd._arrowDown.setVisible(_dictLine < (int)_dictDef.size()-_lines);
	_gd._arrowDown.work();

    _controlsDict.work(input, speedFactor);

}

void PlayGameDict::button(Input* input, IInput::eButtonType b)
{
	switch (b)
	{
	case Input::UP:
		if (input->isPressed(b))
			//move the _dictLine offset var up a line
			scrollDictDown();
		break;
	case Input::DOWN:
		if (input->isPressed(b))
			//move the _dictLine offset var down a line
			scrollDictUp();
		break;
	case Input::Y:
	case Input::CLICK:
//		if (input->isPressed(b)) doDictionary();
        pp_g::pushSDL_Event(USER_EV_EXIT_SUB_SCREEN);
		break;

	default:break;
	}
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
	//check if touch scroll arrows
	if (_gd._arrowUp.contains(pt))
	{
		if (_gd._arrowUp.isTouchable())
			_gd._arrowUp.startAnim(0, -1, ImageAnim::ANI_ONCE, 40);
		scrollDictDown();
	}
	else if (_gd._arrowDown.contains(pt))
	{
		if (_gd._arrowDown.isTouchable())
			_gd._arrowDown.startAnim(0, -1, ImageAnim::ANI_ONCE, 40);
		scrollDictUp();
	}
//disable the tab background feature
//	else
//		if (!_doubleClick.done())
//			doDictionary();
//		else
//			_doubleClick.start(300);

    return true;
}

//releasing 'touch' press
bool PlayGameDict::tap(const Point &pt)
{
    _ctrl_id = _controlsDict.tapped(pt);

    if (_ctrl_id == CTRLID_PREV)
    {
//        doDictionary();
        pp_g::pushSDL_Event(USER_EV_EXIT_SUB_SCREEN);
        return true;
    }

    return false;
}





