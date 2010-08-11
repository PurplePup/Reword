////////////////////////////////////////////////////////////////////
/*

File:			playgamepopup.cpp

Class impl:		PlayGamePopup

Description:	A class derived from the IPlay interface to handle the in-game popup 
				menu to allow "end of level", "next word" or "quit game" options

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			02 Feb 2008

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4		02.02.2008	Created
						11.03.2008	Added Yes/No on quit
				0.5		28.05.2008	Added touchscreen support
						11.08.2008	Build and render menu like PlayMenu class
										but use map instaed of seq container.

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
#include "playgamepopup.h"
#include "audio.h"
#include "platform.h"

#include <string>
#include <cassert>


PlayGamePopup::PlayGamePopup(GameData &gd, bool bMaxWordFound)  : _gd(gd)
{
	_pItems = 0;
	_running = false;
	_init = false;
	_hasMaxWord = bMaxWordFound; //(6 == maxwordlen);
}

//can call setHasMaxWord() first to make sure the menu options are enabled correctly
//if init() called externally again to reset the menu...
void PlayGamePopup::init(Input *input)
{
	//once the class is initialised, init and running are set true

	//Do not init repeat keys so we dont have to reset game repeat on return
	_menuoption = 0;	//always start on 0
	_selectedId = 0;
	_bSelected = false;			//bool for caller to determine if selection made
	_bConfirm = false;			//true goes into yes/no mode if exit pressed
	
	prepareBackground();
	
	memset(_touchArea, 0, sizeof _touchArea );
	memset(_touchYNArea, 0, sizeof _touchYNArea );
	_gd._star.setPos(0,0);	//modified once menu text X pos returned from put_text()
	_gd._star.startAnim( 0, 6, ImageAnim::ANI_LOOP, 35, 0);

	int i(0);
	_itemList.clear();
	_itemList[i=0] = MenuItem(POP_CANCEL, GREEN_COLOUR, "Resume Game", "Continue game");
	_itemList[++i] = MenuItem(POP_SKIP, ORANGE_COLOUR, "Next word/Level", (_hasMaxWord)?"Move on to next word/level":"You must have a 6 first!", _hasMaxWord);
/*	bool bIsPlaying = false, bHasMusic = false;
	Audio * pAudio = Audio::instance();
	if (pAudio)
	{
		bIsPlaying = pAudio->isPlayingMusic();	//may be playing/fading menu music so uses isPlaying... rather than isActuallyPl...
		bHasMusic = pAudio->hasMusicTracks();
	}
	_itemList[++i] = MenuItem(POP_TOGGLEMUSIC, BLUE_COLOUR, bIsPlaying?"Music Stop":"Music Start", bIsPlaying?"Stop current track":"Start a song", bHasMusic);
	_itemList[++i] = MenuItem(POP_NEXTTRACK, BLUE_COLOUR, "Next Track", "Play next song", bHasMusic);
	_itemList[++i] = MenuItem(POP_PREVTRACK, BLUE_COLOUR, "Prev Track", "Play previous song", bHasMusic);
*/	_itemList[++i] = MenuItem(POP_QUIT, RED_COLOUR, "Quit Game !", "Exit (saves if highscore)");
	
	_itemYNList.clear();
	_itemYNList[i=0] = MenuItem(POP_NO, GREEN_COLOUR, "No", "Back to menu");
	_itemYNList[++i] = MenuItem(POP_YES, RED_COLOUR, "Yes", "Quit the game");

	_pItems = &_itemList;
	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

void PlayGamePopup::render(Screen *s)
{
	//draw menu overlay
	int x = (s->_width - _gd._popupmenu.tileW()) / 2;
	int y = (s->_height - _gd._popupmenu.tileH()) / 2;
	_gd._popupmenu.blitTo(s, x, y, 0);
//	_menubg->blitTo(s, 0, 0, 0);

	//calculate best text gap
	int maxGap = _gd._fntSmall.height()*2; //max gap
	int topGap = _gd._fntSmall.height();
	int h = _gd._popupmenu.tileH() - (_gd._fntSmall.height()*3) - 10; //10=5pix*2 border
	int texth = _pItems->size() * _gd._fntSmall.height();
	int gap = (h - texth) / (_pItems->size()-1);
	if (gap > maxGap)
	{
		gap = maxGap;
		topGap += (h - texth - (gap*((*_pItems).size()-1))) /2;
	}
 
	int yy = y + topGap;
	Rect r;
	for (tMenuMap::iterator it = _pItems->begin(); it != _pItems->end(); ++it)
	{
		if (it->first == _menuoption)
		{
			r = _gd._fntSmall.put_text(s, yy, it->second._title.c_str(), (it->second._enabled)?it->second._hoverOn:GREY_COLOUR, true);
			r._min._x -= _gd._star.tileW()+5;
			_gd._star.setPos(r._min._x, yy);
			//help/comment str
			_gd._fntClean.put_text(s, (y+_gd._popupmenu.tileH())-(_gd._fntSmall.height()*2), it->second._comment.c_str(), GREY_COLOUR, true);
		}
		else
		{
			r = _gd._fntSmall.put_text(s, yy, it->second._title.c_str(), (it->second._enabled)?it->second._hoverOff:GREY_COLOUR, false);
		}
		// Make the touch area bigger
		it->second._r = r.inset(-5);
		yy += gap + _gd._fntSmall.height();
	}

	_gd._star.draw(s);
}

void PlayGamePopup::work(Input *input, float speedFactor)
{
	_gd._star.work();

	//Do repeat keys...if a key is pressed and the interval
	//has expired process that button as if pressesd again
    if (input->repeat(Input::UP)) button(input, Input::UP);
    if (input->repeat(Input::DOWN))	button(input, Input::DOWN);
}

void PlayGamePopup::button(Input *input, Input::ButtonType b)
{
	switch (b)
	{
	case Input::UP: 
		if (input->isPressed(b))
		{
			if (0==_menuoption)
				_menuoption=(int)_pItems->size()-1;//(_bConfirm)?(int)_itemYNList.size()-1:(int)_itemList.size()-1;	//down to bottom
			else 
				_menuoption--;
		}
		break;
	case Input::DOWN: 
		if (input->isPressed(b))
		{
			if (_menuoption == (int)_pItems->size()-1)
				_menuoption=0;	//back to top
			else 
				_menuoption++;
		}
		break;
	case Input::CLICK: 
	case Input::B:
		if (input->isPressed(b))
			choose();
		break;

	default:break;
	}
	
}

void PlayGamePopup::touch(Point pt)
{
	Rect r;
	for (tMenuMap::iterator it = _pItems->begin(); it != _pItems->end(); ++it)
	{
		if (it->second._r.contains(pt))
		{
			if (!_doubleClick.done() && (it->first == _menuoption))
				choose();
			else
			{
				_doubleClick.start(300);	//start dbl click timer
				_menuoption = it->first;		//highlight the touched item
			}
			break;
		}
	}
}

void PlayGamePopup::choose()
{
	tMenuMap::iterator it = (*_pItems).find(_menuoption);
	assert(it != _itemList.end());
	MenuItem *pItem = &(it->second);
	assert(pItem);

	if (_bConfirm)
	{
		//already showing YES/NO confirm options, so process...
		if (pItem->_id == POP_NO)
		{
			_pItems = &_itemList;		//point back to first menu, then
			_menuoption = ItemFromId(POP_QUIT); 	//NO selected, back to POP_QUIT
			_bConfirm = false;			//back to POP_CANCEL/SKIP/QUIT menu
			return;
		}
		else
			_menuoption = ItemFromId(POP_QUIT);	//YES selected, so exit
	}
	else
	{
		//first check if (and ignore) disabled menu items
		if ( !pItem->_enabled )
			return;
		//if on exit option, go to yes/no options and wait for yes or no
		if (pItem->_id == POP_QUIT)
		{
			_pItems = &_itemYNList;		//point to new menu, then
			_menuoption = ItemFromId(POP_NO);	//get pos of no option
			_bConfirm = true;
			return;
		}
		if (pItem->_id == POP_TOGGLEMUSIC)
		{
			Audio *p = Audio::getPtr();
			if (p)
			{
				if (Mix_PlayingMusic())
					p->pushStopTrack();
				else
					p->pushNextTrack();
			}
		}
		if (pItem->_id == POP_NEXTTRACK)
		{
			Audio *p = Audio::getPtr();
			if (p) p->pushNextTrack();
		}
		if (pItem->_id == POP_PREVTRACK)
		{
			Audio *p = Audio::getPtr();
			if (p) p->pushPrevTrack();
		}
		
	}
	_bSelected = true;
	_selectedId = pItem->_id;
}

void PlayGamePopup::prepareBackground()
{
//	int w = SCREEN_WIDTH; //-(SCREEN_WIDTH/2);
//	int h = SCREEN_HEIGHT; //-(SCREEN_HEIGHT/2);
//	_menubg = std::auto_ptr<Image>(new Image(w , h, 220));
//	_menubg->drawSolidRectA(0, 0, w, h, WHITE_COLOUR, 220 );
}

int PlayGamePopup::ItemFromId(int id)
{
	for (tMenuMap::iterator it = _pItems->begin(); it != _pItems->end(); ++it)
	{
		if (it->second._id == id) return it->first;
	}
	return 0;
}

