////////////////////////////////////////////////////////////////////
/*

File:			playoptions.cpp

Class impl:		PlayOptions

Description:	A class derived from the IPlay interface to handle all
				game options and preferences such as default word file,
				default difficulty, single touch menus etc

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			21 Jan 2012

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.6		21.01.12	created

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
#include "playoptions.h"
#include "helpers.h"
#include "platform.h"
#include "sprite.h"
#include <sstream>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>


PlayOptions::PlayOptions(GameData &gd)  :
    PlayMenu(gd),
    _yyStart(0), _yyGap(0), _xxStartText(0), _xxStartCtrls(0)
{
//	_running = false;
//	_init = false;
}

PlayOptions::~PlayOptions()
{
    Control *p;

    //save changed settings...

    p = _controlsOptn.getControl(CTRLID_YES_NO);
    if (p) _gd._options.setSingleTap(p->isFirstMode());  //first = yes

    p = _controlsOptn.getControl(CTRLID_FX);
    if (p) _gd._options.setDefaultSfxOn(p->isFirstMode());  //first = on

    p = _controlsOptn.getControl(CTRLID_MUSIC);
    if (p) _gd._options.setDefaultMusicOn(p->isFirstMode());  //first = on

    _gd._options.save();
}

void PlayOptions::init(Input *input)
{
	setTitle("OPTIONS");
	setHelp("Press B to select option", GREY_COLOUR);

    setFont( MENU_FONT_SMALL, MENU_FONT_CLEAN );
	setLayout(MENU_LAYOUT_LEFT, SCREEN_WIDTH/8);
	addItem(MenuItem(0, BLACK_COLOUR, "Preferred wordfile :", "Change language or dictionary file"));
	addItem(MenuItem(1, BLACK_COLOUR, "Single touch menus :", "Single or double tap menus"));
	addItem(MenuItem(2, BLACK_COLOUR, "Default difficulty :", "Always start on this difficulty"));
	addItem(MenuItem(3, BLACK_COLOUR, "Default sound effects :", "Turn on/off in-game effects"));
	addItem(MenuItem(4, BLACK_COLOUR, "Default menu Music :", "Turn on/off menu music"));
	addItem(MenuItem(5, BLACK_COLOUR, "In-game music files :", "Directory where your music is stored"));
	setItem(0);

    _xxStartCtrls = getItemWidest().right() + 20;

    MenuItem wordItem = getItem(0);
    _yyWordFile = wordItem._rBox.top();

    {
    boost::shared_ptr<Sprite> p(new Sprite(RES_IMAGES + "btn_square_yes_no_small.png", 255, 9));
    MenuItem i = getItem(1);
    p->setPos(_xxStartCtrls, i._rBox.top());
    p->_sigEvent.connect(boost::bind(&PlayOptions::ControlEvent, this, _1, _2));
    Control c(p, CTRLID_YES_NO, 0, Control::CAM_DIS_HIT_IDLE_DOUBLE, _gd._options._bSingleTapMenus);
    _controlsOptn.add(c);
    }
    {
    boost::shared_ptr<Sprite> p(new Sprite(RES_IMAGES + "btn_round_fx.png", 255, 9));
    MenuItem i = getItem(3);
    p->setPos(_xxStartCtrls, i._rBox.top());
    p->_sigEvent.connect(boost::bind(&PlayOptions::ControlEvent, this, _1, _2));
    Control c(p, CTRLID_FX, 0, Control::CAM_DIS_HIT_IDLE_DOUBLE, _gd._options._bSfx);
    _controlsOptn.add(c);
    }
    {
    boost::shared_ptr<Sprite> p(new Sprite(RES_IMAGES + "btn_round_music.png", 255, 9));
    MenuItem i = getItem(4);
    p->setPos(_xxStartCtrls, i._rBox.top());
    p->_sigEvent.connect(boost::bind(&PlayOptions::ControlEvent, this, _1, _2));
    Control c(p, CTRLID_MUSIC, 0, Control::CAM_DIS_HIT_IDLE_DOUBLE, _gd._options._bMusic);
    _controlsOptn.add(c);
    }

    //load EXIT/NEXT buttons
    {
    boost::shared_ptr<Sprite> p(new Sprite(RES_IMAGES + "btn_square_exit_small.png", 255, 5));
    p->setPos(8, BG_LINE_BOT + ((SCREEN_HEIGHT - BG_LINE_BOT - p->tileH())/2));
    p->_sigEvent.connect(boost::bind(&PlayOptions::ControlEvent, this, _1, _2));
    Control c(p, CTRLID_EXIT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsOptn.add(c);
    }

    setupWordFile();

	PlayMenu::init(input);
}

void PlayOptions::render(Screen *s)
{
	if (!_init) return;

	PlayMenu::render(s);

    _controlsOptn.render(s);

    //difficulty
	_gd._fntClean.put_text(s, _xxStartCtrls, _yyWordFile, _wordFileList[_wordFileIdx].c_str(), BLACK_COLOUR, false);


}

void PlayOptions::work(Input *input, float speedFactor)
{
    PlayMenu::work(input, speedFactor);

    _controlsOptn.work(input, speedFactor);
}

void PlayOptions::choose(MenuItem i)
{
	switch (i._id) {
		case 0: //preferred wordfile
                _wordFileIdx++;
                if (_wordFileIdx >= (int)_wordFileList.size())
                    _wordFileIdx = 0;
                break;
		case 1: { //single touch
                    Control *p = _controlsOptn.getControl(CTRLID_YES_NO);
                    if (p) p->fade();  //manually switch yes/no
                }
                break;
		case 2: //default diff
                break;
		case 3: { //sound fx
                    Control *p = _controlsOptn.getControl(CTRLID_FX);
                    if (p) p->fade();  //manually switch yes/no
                }
                break;
		case 4: { //music
                    Control *p = _controlsOptn.getControl(CTRLID_MUSIC);
                    if (p) p->fade();  //manually switch yes/no
                }
                break;
		case 5: //music dir
                break;
	}
}

//event signal from imageanim indicating end of animation
void PlayOptions::ControlEvent(int event, int control_id)
{
    if (event == USER_EV_END_ANIMATION)
    {
        if (control_id == CTRLID_EXIT)
        {
            _gd._state = ST_MENU;		//back to menu
            _running = false;
        }
    }
}


void PlayOptions::button(Input *input, ppkey::eButtonType b)
{
    PlayMenu::button(input, b);
//	switch (b)
//	{
//	case ppkey::UP:
//		if (input->isPressed(b))
//			//move the _instLine offset var up a line
//			scrollDown();
//		break;
//	case ppkey::DOWN:
//		if (input->isPressed(b))
//			//move the _instLine offset var down a line
//			scrollUp();
//		break;
//	case ppkey::Y:
//	case ppkey::START:
//		if (input->isPressed(b))	//exit back to menu
//		{
//			_gd._state = ST_MENU;
//			_running = false;	//exit this class running state
//		}
//		break;
//	case ppkey::CLICK:
//	case ppkey::B:
//		if (input->isPressed(b))
//			nextPage();
//		break;
//	default:break;
//	}
//
//    updateScrollButtons();
}

bool PlayOptions::touch(const Point &pt)
{
    const int crtl_id = _controlsOptn.touched(pt);    //needed to highlight a touched control

    PlayMenu::touch(pt);

	return false;
}

//releasing 'touch' press
bool PlayOptions::tap(const Point &pt)
{
    const int crtl_id = _controlsOptn.tapped(pt);

    PlayMenu::tap(pt);

//
//    if (crtl_id == CTRLID_NEXT)
//    {
//        nextPage();
//        return true;
//    }

    return false;
}

void PlayOptions::setupWordFile()
{
    _wordFileList.clear();
    _wordFileIdx = 0;

    boost::filesystem::path dir( RES_WORDS );

    boost::filesystem::directory_iterator end;
    int idx = 0;
    for (boost::filesystem::directory_iterator pos(dir); pos!=end; ++pos, ++idx)
    {
        if ( is_regular_file( *pos ) )
        {
            if (pos->path().filename() == _gd._options._defaultWordFile)
            {
                _wordFileIdx = idx;
            }
            _wordFileList.push_back(pos->path().filename());
//            std::cout << pos->path().filename() << " : " << boost::filesystem::file_size( pos->path() ) << "\n";
        }
    }
    if (_wordFileList.empty())
        _wordFileList.push_back("<no word files>");

}

