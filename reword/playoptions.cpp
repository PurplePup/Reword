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
				0.7		02.01.17	Moved to SDL2
				0.7		02.01.17	Moved to SDL2

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
#include "signal.h"
#include "resource.h"
#include "locator.h"

#include <sstream>
#include <filesystem>


PlayOptions::PlayOptions(GameData &gd)  :
    PlayMenu(gd),
    _yyStart(0), _yyGap(0), _xxStartText(0), _xxStartCtrls(0)
{
}

PlayOptions::~PlayOptions()
{
    Control *p;

    //save changed settings...

    if (_gd._options.setDefaultWordFile(_wordFileList[_wordFileIdx]))
    {
        //load newly selected word file
        _gd._words.load(RES_WORDS + _gd._options._defaultWordFile, SDL_GetTicks());
        //load scores for this wordfile
        _gd._score.loadUsingWordfileName(_gd._options._defaultWordFile);
    }

    p = _controlsOptn.getControl(CTRLID_YES_NO);
    if (p) _gd._options.setSingleTap(p->isFirstState());  //first = yes

    p = _controlsOptn.getControl(CTRLID_DIFF);
    if (p) _gd._options.setDefaultDiff((eGameDiff)p->currState());  //first = easy(0) med(1) hard(2)

    p = _controlsOptn.getControl(CTRLID_SFX);
    if (p) _gd._options.setDefaultSfxOn(p->isFirstState());  //first = yes (on)

    p = _controlsOptn.getControl(CTRLID_MUSIC);
    if (p) _gd._options.setDefaultMusicOn(p->isFirstState());  //first = yes (on)

    _gd._options.save();
}

void PlayOptions::init(Input *input, Screen * scr)
{
	setName("OPTIONS");
    std::string msg = "Press " + input->keyDescription(ppkey::B) + " to select option";
	setHelp(msg, GREY_COLOUR);

	setLayout(PlayMenu::LAYOUT_LEFT, SCREEN_WIDTH/8);
    //setFont( MenuItem::MENU_FONT_SMALL, MenuItem::MENU_FONT_CLEAN );
	addItem(MenuItem(0, BLACK_COLOUR, "Preferred wordfile :", "Change language or dictionary file"));
	addItem(MenuItem(1, BLACK_COLOUR, "Single touch menus :", "Single (else double) tap menus"));
	addItem(MenuItem(2, BLACK_COLOUR, "Default difficulty :", "Use this difficulty at startup"));
	addItem(MenuItem(3, BLACK_COLOUR, "Default Sound FX :", "Turn on in-game effects at startup"));
	addItem(MenuItem(4, BLACK_COLOUR, "Default Menu Music :", "Turn on menu music at startup"));
//	addItem(MenuItem(5, BLACK_COLOUR, "In-game music files :", "Directory where your music is stored"));
	setItem(0);

    _xxStartCtrls = getItemWidest().right() + 20;

    MenuItem wordItem = getItem(0);
    _yyWordFile = wordItem._rBox.top() + (wordItem._r.height()/4);

    {
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_yes_no_small.png")));
    MenuItem i = getItem(1);
    p->setPos((float)_xxStartCtrls, (float)i._rBox.top());
    p->_sigEvent2.Connect(this, &PlayOptions::ControlEvent);
    Control c(p, CTRLID_YES_NO, 0, Control::CAM_DIS_HIT_IDLE_DOUBLE, _gd._options._bSingleTapMenus?1:2);
    _controlsOptn.add(c);
    }
    {
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_diff.png")));
    MenuItem i = getItem(2);
    p->setPos((float)_xxStartCtrls, (float)i._rBox.top());
    p->_sigEvent2.Connect(this, &PlayOptions::ControlEvent);
    Control c(p, CTRLID_DIFF, 0, Control::CAM_DIS_HIT_IDLE_TRIPPLE, (unsigned int)_gd._options._defaultDifficulty);
    _controlsOptn.add(c);
    }
    {
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_yes_no_small.png")));
    MenuItem i = getItem(3);
    p->setPos((float)_xxStartCtrls, (float)i._rBox.top());
    p->_sigEvent2.Connect(this, &PlayOptions::ControlEvent);
    Control c(p, CTRLID_SFX, 0, Control::CAM_DIS_HIT_IDLE_DOUBLE, _gd._options._bDefaultSfxOn?1:2);
    _controlsOptn.add(c);
    }
    {
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_yes_no_small.png")));
    MenuItem i = getItem(4);
    p->setPos((float)_xxStartCtrls, (float)i._rBox.top());
    p->_sigEvent2.Connect(this, &PlayOptions::ControlEvent);
    Control c(p, CTRLID_MUSIC, 0, Control::CAM_DIS_HIT_IDLE_DOUBLE, _gd._options._bDefaultMusicOn?1:2);
    _controlsOptn.add(c);
    }

    {//load EXIT/NEXT buttons
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_exit_small.png")));
    p->setPos(8.0, (float)(BG_LINE_BOT + ((SCREEN_HEIGHT - BG_LINE_BOT - p->tileH())/2)));
    p->_sigEvent2.Connect(this, &PlayOptions::ControlEvent);
    Control c(p, CTRLID_EXIT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsOptn.add(c);
    }

    setupWordFile();

	PlayMenu::init(input, scr);
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
    //make buttons flash/fade when menu items selected to show selection
	switch (i.id()) {
	    case -1:    //exit (using kbd Y)
                _gd._state = ST_MENU;		//back to menu
                _running = false;
                break;
		case 0: //preferred wordfile
                _wordFileIdx++;
                if (_wordFileIdx >= (int)_wordFileList.size())
                    _wordFileIdx = 0;
                break;
		case 1: { //single touch
                    _controlsOptn.forceFade(CTRLID_YES_NO); //manually switch yes/no
                }
                break;
		case 2: { //default diff
                    _controlsOptn.forceFade(CTRLID_DIFF);   //manually switch yes/no
                }
                break;
		case 3: { //sound fx
                    //manually call fade() as we need the control state for the audio
                    Control *p = _controlsOptn.getControl(CTRLID_SFX);
                    if (p)
                    {
                        p->fade();
						const auto bOn = p->isFirstState();
                        Locator::audio().setSfxEnabled(bOn);
                    }
                }
                break;
		case 4: { //music
                    //manually call fade() as we need the control state for the audio
                    Control *p = _controlsOptn.getControl(CTRLID_MUSIC);
                    if (p)
                    {
                        p->fade();
						const auto bOn = p->isFirstState();
						Locator::audio().setMusicEnabled(bOn);
                    }
                }
                break;
		case 5: //music dir
                break;

        default:break;
	}
}

//event signal from imageanim indicating end of animation
void PlayOptions::ControlEvent(int event, int ctrl_id)
{
    (void)(ctrl_id);    //unused

    if (event == USER_EV_END_ANIMATION)
    {
//        if (ctrl_id == CTRLID_EXIT)
//        {
//            _gd._state = ST_MENU;		//back to menu
//            _running = false;
//        }
    }
}


bool PlayOptions::button(Input *input, ppkey::eButtonType b)
{
    return PlayMenu::button(input, b);
}

bool PlayOptions::touch(const Point &pt)
{
    PlayMenu::touch(pt);

    //needed to highlight a touched control (ctrl_id return val not used here)
    _controlsOptn.touched(pt);

	return false;
}

//releasing 'touch' press
bool PlayOptions::tap(const Point &pt)
{
    PlayMenu::tap(pt);

    const int ctrl_id = _controlsOptn.tapped(pt);

    if (ctrl_id == CTRLID_EXIT)
    {
        _gd._state = ST_MENU;		//back to menu
        _running = false;
        return true;
    }

    return false;
}

void PlayOptions::setupWordFile()
{
    _wordFileList.clear();
    _wordFileIdx = 0;

    std::filesystem::path dir( RES_WORDS );

    std::filesystem::directory_iterator end;
    int idx = 0;
    for (std::filesystem::directory_iterator pos(dir); pos!=end; ++pos, ++idx)
    {
        if ( is_regular_file( *pos ) )
        {
            if (pos->path().filename() == _gd._options._defaultWordFile)
            {
                _wordFileIdx = idx;
            }
#ifdef WIN32
            _wordFileList.push_back(pos->path().filename().generic_string());
#else
            _wordFileList.push_back(pos->path().filename().c_str());
#endif
//            std::cout << pos->path().filename() << " : " << STD::filesystem::file_size( pos->path() ) << "\n";
        }
    }
    if (_wordFileList.empty())
        _wordFileList.push_back("<no word files>");

}

