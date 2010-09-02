////////////////////////////////////////////////////////////////////
/*

File:			PlayMenu.cpp

Class impl:		PlayMenu

Description:	A class derived from the IPlay interface to handle all screen
				events and drawing of the Menu screen

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3.1	07.06.2007	Speed up menu movement a little
				0.5.0	18.06.2008	Added code to create menu items dynamically 
										and support touchscreen

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
#include "playmenu.h"
#include "platform.h"
#include <string>

PlayMenu::PlayMenu(GameData &gd)  : _gd(gd)
{
	_init = false;
	_running = false;
	_item = 0;
	_nextYpos = 0;
}

void PlayMenu::init(Input *input)
{
	//once the class is initialised, init and running are set true

	//play menu music - if not already playing
	if (!Mix_PlayingMusic())
		Mix_PlayMusic(_gd._musicMenu, -1);	//play 'forever'

	//set the repeat of the keys required
	input->setRepeat(Input::UP, 150, 300);		//button, rate, delay
	input->setRepeat(Input::DOWN, 150, 300);

	_title.startMoveFrom( 0, -(_gd._letters.tileH()*2), 15, 100, 0, ROUNDEL_VEL);
	_titleW.start(3000, 1000);

	_gd._star.setPos(MENU_HI_X,0);//MENU_HI_Y+(_item*MENU_HI_GAP));	//modified once menu text X pos returned from put_text()
	_gd._star.startAnim( 0, 6, ImageAnim::ANI_LOOP, 35, 0);

	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

void PlayMenu::choose(MenuItem i)
{
	return;
}

void PlayMenu::render(Screen *s)
{
	if (!_init) return;

	_gd._menubg.blitTo( s );

	_title.draw(s);

	_gd._fntTiny.put_text_right(s, 5, VERSION_STRING, BLACK_COLOUR); //display vN.N at top right

	int selected = getSelected()._id;
	int y = MENU_HI_Y+MENU_HI_OFF;
	Rect r;
	for (tMenuItems::iterator p = _itemList.begin(); p != _itemList.end(); p++)
	{
		if (p->_id == selected)
		{
			r = _gd._fntMed.put_text(s, y-2, p->_title.c_str(), p->_hoverOn, true);
			r._min._x -= 30;
			_gd._star.setPos(r._min._x, y-2);
			//show comment for selected item - might be blank
			_gd._fntClean.put_text(s, BG_LINE_BOT - (_gd._fntClean.height()+2) , p->_comment.c_str(), BLACK_COLOUR, false);
		}
		else
		{
			r = _gd._fntMed.put_text(s, y, p->_title.c_str(), p->_hoverOff, false);
		}
		// Make the touch area bigger
		p->_r = r.inset(-5);
		y += MENU_HI_GAP;
	}
	_nextYpos = y;	//useful for placing help text after items
	
	_gd._star.draw(s);

	int helpYpos = BG_LINE_BOT+((SCREEN_HEIGHT-BG_LINE_BOT-_gd._fntClean.height())/2);
	_gd._fntClean.put_text(s, helpYpos, _help.c_str(), _helpColor, true);

}

void PlayMenu::work(Input *input, float speedFactor)
{
	_title.work();
	_gd._star.work();

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

    if (input->repeat(Input::UP)) button(input, Input::UP);
    if (input->repeat(Input::DOWN))	button(input, Input::DOWN);
}

void PlayMenu::button(Input *input, Input::ButtonType b)
{
	switch (b)
	{
	case Input::UP: 
		if (input->isPressed(b) && _itemList.size())
		{
			if (0==_item)
				_item=_itemList.size()-1;	//down to bottom
			else
				_item--;
		}
		break;
	case Input::DOWN: 
		if (input->isPressed(b) && _itemList.size())
		{
			if (_itemList.size()-1==_item)
				_item=0;	//back to top
			else
				_item++;
		}
		break;
	case Input::SELECT:
	case Input::CLICK: 
	case Input::START:
	case Input::B:
		if (input->isPressed(b) && _itemList[_item]._enabled)
			choose(_itemList[_item]);
		break;

	default:break;
	}
}

void PlayMenu::touch(Point pt)
{
	Uint32 item(0);
	for (tMenuItems::iterator p = _itemList.begin(); p != _itemList.end(); ++item, ++p) {
		if (p->_enabled && p->_r.contains(pt))
		{
			//double click to select the item, or if a single click, highlight the item,
			if (!_doubleClick.done() && item == _item)
				choose(*p);
			else
			{
				_doubleClick.start(300);	//start dbl click timer
				_item = item;	//highlight the touched item
			}
			return;
		}
	}
}

void PlayMenu::setTitle(std::string title)
{
	_title.setWordCenterHoriz(title, _gd._letters, (BG_LINE_TOP-_gd._letters.tileH())/2, 2);
}

void PlayMenu::setHelp(std::string help, SDL_Color c)
{
	_help = help;
	_helpColor = c;
}

Uint32 PlayMenu::addItem(MenuItem mi)
{
	_itemList.push_back(mi);
	return _itemList.size();	//return new no. of items in menu
}

void PlayMenu::setItem(int item)
{
	for (unsigned int i=0; i<_itemList.size(); i++)
		if (_itemList[i]._id == item)
			_item = i;
}

MenuItem PlayMenu::getItem(int item)
{
	for (unsigned int i=0; i<_itemList.size(); i++)
		if (_itemList[i]._id == item)
			return _itemList[i];
	return MenuItem();
}

MenuItem PlayMenu::getSelected()
{
	return _itemList[_item];
}


