////////////////////////////////////////////////////////////////////
/*

File:			PlayMenu.cpp

Class impl:		PlayMenu

Description:	A class derived from the IPlay interface to handle all screen
				events and drawing of any generic Menu screens

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3.1	07.06.2007	Speed up menu movement a little
				0.5.0	18.06.2008	Added code to create menu items dynamically
										and support touchscreen
                0.6.0   02.07.2011  Make single touch menu operation (not double click/tap)

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
#include "audio.h"
#include "locator.h"
#include "sprite.h"

#include <string>

PlayMenu::PlayMenu(GameData &gd)  : _gd(gd)
{
	_init = false;
	_running = false;
	_item = 0;
	_nextYpos = 0;
	_menuRect = Rect(0, BG_LINE_TOP, SCREEN_WIDTH, BG_LINE_BOT);
	_layoutType = MENU_LAYOUT_CENTER;
	_layoutOffset = 0;
	_font = &_gd._fntMed;
    _fontHelp = &_gd._fntClean;
	_bSetStarPos = true;
}

void PlayMenu::init(Input *input)
{
	//once the class is initialised, init and running are set true

    ppg::pushSDL_Event(USER_EV_START_MENU_MUSIC);

	//set the repeat of the keys required
	input->setRepeat(ppkey::UP, 150, 300);		//button, rate, delay
	input->setRepeat(ppkey::DOWN, 150, 300);

    tSharedImage &letters = Resource::image("roundel_letters.png");
	_title.easeMoveFrom( 0, -(letters->height()*2), 1000, -500);
	_titleW.start(3000, 1000);

    _menubg = Resource::image("menubg.png");

	_star.setImage(Resource::image("star.png"));
	_star.setPos(MENU_HI_X,0); //modified once menu text X pos returned from put_text()
	_star.startAnim( 0, 6, ImageAnim::ANI_LOOP, 35, 0);
	_star.setObjectId(CTRLID_STAR);

    //music on/off icon
    { // round music button placed in top left of scorebar
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_music.png")));
    p->setPos(5,5);
    IAudio &audio = Locator::audio();
    Control c(p, CTRLID_MUSIC, 0, Control::CAM_DIS_HIT_IDLE_DOUBLE, audio.musicEnabled()?1:2);
    _controlsMenu.add(c);
    _controlsMenu.enableControl(audio.hasSound(), CTRLID_MUSIC);  //disable override?
    }

    tSharedImage &let = Resource::image("roundel_kbd_letters.png");
	_beta.setWord("BETA", let, Screen::width() - ((let->tileW()+2)*4), Screen::height() - let->tileH(), 2);
	_beta.easeMoveFrom( 0, Screen::height(), 800, -1000, Easing::EASE_OUTQUART);

	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

void PlayMenu::setFont(eMenuFont mainFont, eMenuFont helpFont)
{
    switch (mainFont)
    {
        case MENU_FONT_SMALL: _font = &_gd._fntSmall; break;
        case MENU_FONT_MED: _font = &_gd._fntMed; break;
        case MENU_FONT_BIG: _font = &_gd._fntBig; break;
        default: _font = &_gd._fntMed; break;
    }
    switch (helpFont)
    {
        case MENU_FONT_SMALL: _fontHelp = &_gd._fntSmall; break;
        case MENU_FONT_MED: _fontHelp = &_gd._fntMed; break;
        case MENU_FONT_BIG: _fontHelp = &_gd._fntBig; break;
        default: _fontHelp = &_gd._fntClean; break;
    }
}

void PlayMenu::setMenuArea(const Rect &r)
{
    _menuRect = r;
    recalcItemPositions();
}

//function to be overloaded in derived classes to process choice
void PlayMenu::choose(MenuItem i)
{
    (void)(i);
	return;
}

void PlayMenu::chooseDone()
{
    //stub, to be overridden and used by actual menu
    //to detect end of menu (star moving off screen)
    //so animations can finish before moving to next
    //state
}

void PlayMenu::render(Screen *s)
{
	if (!_init) return;

	//_gd._menubg.blitTo( s );
	//ppg::blit_surface(_gd._menubg.surface(), NULL, s->surface(), 0, 0);
	ppg::blit_surface(_menubg->surface(), NULL, s->surface(), 0, 0);

	_title.render(s);
	_beta.render(s);

	int selected = getSelected()._id;
	int nextY = 0;
	Rect r;
	for (tMenuItems::iterator p = _itemList.begin(); p != _itemList.end(); p++)
	{
		if (p->_id == selected)
		{
            _font->put_text(s, p->_r.left(), p->_r.top(), p->_title.c_str(), p->_hoverOn, true);
            if (_bSetStarPos)
            {
                r = p->_r.addpt(Point(-30,0));
                _star.setPos(r.left(), r.top()-2);
                _bSetStarPos = false;
            }

			//show comment for selected item - might be blank
            if (_delayHelp.done())
            {
                _fontHelp->put_text(s,
                        BG_LINE_BOT + (((SCREEN_HEIGHT-BG_LINE_BOT-(_fontHelp->height()*2))/4)),
                        p->_comment.c_str(), p->_hoverOn, false);
            }
		}
		else
		{
            _font->put_text(s, p->_r.left(), p->_r.top(), p->_title.c_str(), p->_hoverOff, false);
		}
        nextY = p->_r.bottom();
	}
	_nextYpos = nextY;	//useful for placing help text after all items drawn

	_star.draw(s);
    _controlsMenu.render(s);

    int helpYpos = BG_LINE_BOT + (2*( (SCREEN_HEIGHT-BG_LINE_BOT-_fontHelp->height())/2.5 ));
    _fontHelp->put_text(s, helpYpos, _help.c_str(), _helpColor, true);

}

void PlayMenu::work(Input *input, float speedFactor)
{
    (void)(speedFactor);

	_title.work(input, speedFactor);
	_star.work();

	_beta.work(input, speedFactor);

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

    if (input->repeat(ppkey::UP)) button(input, ppkey::UP);
    if (input->repeat(ppkey::DOWN))	button(input, ppkey::DOWN);

    _controlsMenu.work(input, speedFactor);
}

bool PlayMenu::button(Input *input, ppkey::eButtonType b)
{
	switch (b)
	{
	case ppkey::UP:
		if (input->isPressed(b) && _itemList.size())
		{
            _bSetStarPos = true;
			if (0==_item)
				_item=_itemList.size()-1;	//down to bottom
			else
				_item--;
		}
		break;
	case ppkey::DOWN:
		if (input->isPressed(b) && _itemList.size())
		{
            _bSetStarPos = true;
			if (_itemList.size()-1==_item)
				_item=0;	//back to top
			else
				_item++;
		}
		break;
	case ppkey::SELECT:
	case ppkey::CLICK:
	case ppkey::START:
	case ppkey::B:
	case ppkey::X:
		if (input->isPressed(b) && _itemList[_item]._enabled)
			choose(_itemList[_item]);
		break;
    case ppkey::Y:
        if (input->isPressed(b))
            choose(MenuItem()); //id = -1 == keyboard exit
        break;

	default:return false;
	}
	return true;
}

//touch (press) to highlight the menu item
bool PlayMenu::touch(const Point &pt)
{
    /*const int ctrl_id = */ _controlsMenu.touched(pt);

    _saveTouchPt._x = _saveTouchPt._y = 0;
	Uint32 item(0);

	for (tMenuItems::iterator it = _itemList.begin(); it != _itemList.end(); ++item, ++it)
	{
		if (it->_enabled && it->_rBox.contains(pt))
		{
            _bSetStarPos = true;
            if (_gd._options._bSingleTapMenus)
            {
                _saveTouchPt = pt;      //so test if release pos is in same menu item
                _item = item;	        //highlight the touched item
                _delayHelp.start(500);  //wait before help line displayed
                return true;
            }
            else
            {
                if (!_doubleClick.done() && (item == _item))
                    choose(*it);
                else
                {
                    _doubleClick.start(300);	//start dbl click timer
                    _item = item;		//highlight the touched item
                }
                return true;
            }
		}
	}
    return false;
}

//tap (release) to action the menu item
bool PlayMenu::tap(const Point &pt)
{
    const int ctrl_id = _controlsMenu.tapped(pt);

    if (_gd._options._bSingleTapMenus)
    {
        Uint32 item(0);
        for (tMenuItems::iterator it = _itemList.begin(); it != _itemList.end(); ++item, ++it) {
            if (it->_enabled && it->_rBox.contains(pt))
            {
                if (it->_rBox.contains(_saveTouchPt))   //is same control originally touched (ie not move away to cancel)
                {
                    _saveTouchPt = Point();
                    choose(*it);
                }
                return true;
            }
        }
    }

    //game music icon action on press, not tap(release)
    if (ctrl_id == CTRLID_MUSIC)// && Locator::audio().musicEnabled())
    {
	    Locator::audio().toggleMusic(true);
        return true;
    }

	return false;
}

void PlayMenu::handleEvent(SDL_Event &sdlevent)
{
	if (sdlevent.type == SDL_USEREVENT)
	{
		if (USER_EV_END_MOVEMENT == sdlevent.user.code)
		{
		    //USER_EV_END_MOVEMENT uses data1 as a simple int(as a ptr to sprite
            //internal data may not exist after end of movement).
            const int id = reinterpret_cast<int>(sdlevent.user.data1);
            if (id == CTRLID_STAR)
            {
                chooseDone();   //tell super that star has gone
            }
		}
	}
}

void PlayMenu::setTitle(const std::string &title)
{
    tSharedImage &letters = Resource::image("roundel_letters.png");
	_title.setWordCenterHoriz(title, letters, (BG_LINE_TOP-letters->height())/2, 2);
}

void PlayMenu::setHelp(const std::string &help, SDL_Color c)
{
	_help = help;
	_helpColor = c;
}

//calc each item pos based on number of curr loaded items and
//any min/max (top/bot) placing points
void PlayMenu::recalcItemPositions()
{
    const int items = (int)_itemList.size();
    const int freeHeight = (_menuRect.height()) - (items * _font->height());
    const int gapHeight = freeHeight / (items + 1);
    int y = _menuRect.top() + gapHeight;
	for (int i=0; i<items; i++)
    {
        const int len = _font->calc_text_length(_itemList[i]._title.c_str(), false);
        int x;
        if (_layoutType == MENU_LAYOUT_LEFT)
        {
            x = _layoutOffset;
        }
        else if (_layoutType == MENU_LAYOUT_RIGHT)
        {
            x = (SCREEN_WIDTH-len) + _layoutOffset;
        }
        else
            x = (SCREEN_WIDTH-len) / 2;   //CENTER

        _itemList[i]._r = Rect(x, y, x+len, y+_font->height());
        _itemList[i]._rBox = _itemList[i]._r.inset(-5);
        y += gapHeight + _font->height();
    }
}

Uint32 PlayMenu::addItem(MenuItem mi)
{
	_itemList.push_back(mi);
    recalcItemPositions();
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

void PlayMenu::setLayout(eMenuLayout layoutType, int offset)
{
    _layoutType = layoutType;
    _layoutOffset = offset;
    recalcItemPositions();
}

Rect PlayMenu::getItemWidest()
{
    Rect r;
	for (unsigned int i=0; i<_itemList.size(); i++)
	{
        if (_itemList[i]._r.width() > r.width())
            r = _itemList[i]._r;
	}
    return r;
}

//cleanup anims on exit (esp main menu - exit app - screen freeze)
//so remove animating controls etc. Just a little tidier than freeze
void PlayMenu::exitMenu()
{
//    _controlsMenu.showAllControls(false);


    const int ms = 1200;
	const int btnWidth = _star.tileW();
    _star.startMoveTo(-btnWidth, _star.getYPos(), ms);
//    _star.startMoveTo(10, 100, 20, 0, 8, 0);

}
