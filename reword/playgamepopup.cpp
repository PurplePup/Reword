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
										but use map instead of seq container.
                0.6     20.06.2011  Fix Yes/No menu exit

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
#include "score.h"

#include <string>
#include <cassert>


PlayGamePopup::PlayGamePopup(GameData &gd, bool bIsInGame, bool bMaxWordFound)  : _gd(gd)
{
	_pItems = 0;
	_running = false;
	_init = false;
	_bIsInGame = bIsInGame;
	_hasMaxWord = bMaxWordFound; //(6 == maxwordlen);
}

//can call setHasMaxWord() first to make sure the menu options are enabled correctly
//if init() called externally again to reset the menu...
void PlayGamePopup::init(Input *input)
{
    (void)(input);

	//once the class is initialised, init and running are set true

	//Do not init repeat keys so we dont have to reset game repeat on return
	_menuoption = 0;	//always start on 0
	_selectedId = 0;
	_bSelected = false;		//bool for caller to determine if selection made
	_bDoYesNoMenu = false;		//true goes into yes/no mode if exit pressed

	prepareBackground();

	memset(_touchArea, 0, sizeof _touchArea );
	memset(_touchYNArea, 0, sizeof _touchYNArea );

	_star.setImage(Resource::image("star.png"));
	_star.setPos(0,0);	//modified once menu text X pos returned from put_text()
	_star.startAnim( 0, 6, ImageAnim::ANI_LOOP, 35, 0);

	int i(0);
	//main (in-game) menu options
	_itemList.clear();
	_itemList[i=0] = MenuItem(POP_CANCEL, GREEN_COLOUR, "Resume Game", "Continue game");
	if (_bIsInGame)
        _itemList[++i] = MenuItem(POP_SKIP, ORANGE_COLOUR, "Next word/Level", (_hasMaxWord)?"Move on to next word/level":"You must have a Re-word first!", _hasMaxWord);
    if (Locator::audio().musicEnabled())
    {
        bool bIsPlaying = Locator::audio().isPlayingMusic();	//may be playing/fading menu music so uses isPlaying... rather than isActuallyPl...
        bool bHasMusic = Locator::audio().hasMusicTracks();

        _itemList[++i] = MenuItem(POP_TOGGLEMUSIC, BLUE_COLOUR, bIsPlaying?"Music Stop":"Music Start", bIsPlaying?"Stop current track":"Start a song", bHasMusic);
        _itemList[++i] = MenuItem(POP_NEXTTRACK, BLUE_COLOUR, "Next Track", "Play next song", bHasMusic);
        _itemList[++i] = MenuItem(POP_PREVTRACK, BLUE_COLOUR, "Prev Track", "Play previous song", bHasMusic);
    }
	if (_hasMaxWord)
		_itemList[++i] = MenuItem(POP_SAVE, BLUE_COLOUR, "Save & Exit", "Allows exit and restart at same place");
	_itemList[++i] = MenuItem(POP_QUIT, RED_COLOUR, "Quit Game !", "Exit (save highscore)");

    //confirmation options
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
	const int x = (s->_width - _menubg->width()) / 2;
	const int y = (s->_height - _menubg->height()) / 2;
//	_menubg->blitTo(s, x, y, 0);
	ppg::blit_surface(_menubg->surface(), NULL, s->surface(), x, y);

	//calculate best text gap
	const int maxGap = _gd._fntSmall.height()*2; //max gap
	int topGap = _gd._fntSmall.height();
	const int h = _menubg->height() - (_gd._fntSmall.height()*3) - 10; //10=5pix*2 border
	const int texth = _pItems->size() * _gd._fntSmall.height();
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
			r._min._x -= _star.tileW()+5;
			_star.setPos(r._min._x, yy);
			//help/comment str
			_gd._fntClean.put_text(s, (y+_menubg->height())-(_gd._fntSmall.height()*2), it->second._comment.c_str(), GREY_COLOUR, true);
		}
		else
		{
			r = _gd._fntSmall.put_text(s, yy, it->second._title.c_str(), (it->second._enabled)?it->second._hoverOff:GREY_COLOUR, false);
		}
		// Make the touch area bigger
		it->second._r = r.inset(-5);
		yy += gap + _gd._fntSmall.height();
	}

	_star.draw(s);
}

void PlayGamePopup::work(Input *input, float speedFactor)
{
    (void)(speedFactor);

	_star.work();

	//Do repeat keys...if a key is pressed and the interval
	//has expired process that button as if pressesd again
    if (input->repeat(ppkey::UP)) button(input, ppkey::UP);
    if (input->repeat(ppkey::DOWN))	button(input, ppkey::DOWN);
}

bool PlayGamePopup::button(Input *input, ppkey::eButtonType b)
{
	switch (b)
	{
	case ppkey::UP:
		if (input->isPressed(b))
		{
			if (0==_menuoption)
				_menuoption=(int)_pItems->size()-1; //down to bottom
			else
				_menuoption--;
		}
		break;
	case ppkey::DOWN:
		if (input->isPressed(b))
		{
			if (_menuoption == (int)_pItems->size()-1)
				_menuoption=0;	//back to top
			else
				_menuoption++;
		}
		break;
	case ppkey::CLICK:
	case ppkey::B:
		if (input->isPressed(b))
			choose();
		break;

	default:return false;
	}
	return true;
}

bool PlayGamePopup::touch(const Point &pt)
{
	Rect r;
	for (tMenuMap::iterator it = _pItems->begin(); it != _pItems->end(); ++it)
	{
		if (it->second._r.contains(pt))
		{
		    if (_gd._options._bSingleTapMenus)
		    {
				_menuoption = it->first;		//highlight the touched item
                return true;
		    }

			if (!_doubleClick.done() && (it->first == _menuoption))
				choose();
			else
			{
				_doubleClick.start(300);	//start dbl click timer
				_menuoption = it->first;		//highlight the touched item
			}
			return true;
		}
	}
	return false;
}

//tap (release) to action the menu item
bool PlayGamePopup::tap(const Point &pt)
{
    if (_gd._options._bSingleTapMenus)
    {
        Uint32 item(0);
        for (tMenuMap::iterator it = _pItems->begin(); it != _pItems->end(); ++item, ++it) {
            if (it->second._enabled && it->second._r.contains(pt))
            {
                //is same control originally touched (ie not moved away to cancel)
                if (it->first == _menuoption)
                {
                    choose();
                }
                return true;
            }
        }
    }
    return false;
}

void PlayGamePopup::choose()
{
	tMenuMap::iterator it = (*_pItems).find(_menuoption);
	assert(it != (*_pItems).end());
	MenuItem *pItem = &(it->second);
	assert(pItem);

	if (_bDoYesNoMenu)
	{
		//already showing YES/NO confirm options, so process...
		if (pItem->_id == POP_NO)
		{
			_pItems = &_itemList;		//point back to first menu, then
			_menuoption = ItemFromId(POP_QUIT); 	//NO selected, back to POP_QUIT
			_bDoYesNoMenu = false;			//back to POP_CANCEL/SKIP/QUIT menu
			return;
		}
		else
        {
			_pItems = &_itemList;		//point back to first menu, then
			_menuoption = ItemFromId(POP_QUIT);	//YES selected, so exit
            _bSelected = true;
            _selectedId = POP_QUIT;
            return;
        }
	}
	else
	{
		//first check if (and ignore) disabled menu items
		if ( !pItem->_enabled )
			return;
		if (pItem->_id == POP_SAVE)	//for later RESUME
		{
			_menuoption = ItemFromId(POP_QUIT);	//auto (force) exit
		}
		//if on exit option, go to yes/no options and wait for yes or no
		if (pItem->_id == POP_QUIT)
		{
			//setup Yes/No sub menu - to check user actually want to exit
			_pItems = &_itemYNList;		//point to new menu, then
			_menuoption = ItemFromId(POP_NO);	//get pos of no option
			_bDoYesNoMenu = true;
			return;
		}
		if (pItem->_id == POP_TOGGLEMUSIC)
		{
            if (Mix_PlayingMusic())
                Locator::audio().stopTrack();
            else
                Locator::audio().startNextTrack();
		}
		if (pItem->_id == POP_NEXTTRACK)
		{
            Locator::audio().startNextTrack();
		}
		if (pItem->_id == POP_PREVTRACK)
		{
            Locator::audio().startPrevTrack();
		}
	}
	_bSelected = true;
	_selectedId = pItem->_id;
}

void PlayGamePopup::prepareBackground()
{
//	_menubg = tSharedImage(new Image());
//	_menubg->createThisFromImage(_gd._gamemenu);	//in game popupmenu
   	_menubg = Resource::image("popup_menu.png");
}

int PlayGamePopup::ItemFromId(int id)
{
	for (tMenuMap::iterator it = _pItems->begin(); it != _pItems->end(); ++it)
	{
		if (it->second._id == id) return it->first;
	}
	return 0;
}

