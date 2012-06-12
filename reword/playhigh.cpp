////////////////////////////////////////////////////////////////////
/*

File:			playhigh.cpp

Class impl:		PlayHigh

Description:	A class derived from the IPlay interface to handle all screen
				events and drawing of the Hi-Score screen

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4		06.03.2008	Added speeder and timetrial displays and mode logos
				0.5		28.05.08	Added touchscreen support
				0.5.1	08.12.08	Pandora screen size support

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
#include "playhigh.h"
#include "platform.h"
#include "resource.h"
#include "utils.h"
#include "signal.h"

#include <cassert>
#include <memory>

PlayHigh::PlayHigh(GameData &gd)  : _gd(gd)
{

	_running = false;
	_init = false;
}

void PlayHigh::init(Input *input)
{
	//once the class is initialised, init and running are set true

	//play menu music - if not already playing - as we could come
	//here from the end of the game rather than from the menu
    ppg::pushSDL_Event(USER_EV_START_MENU_MUSIC);

	//set up local difficulty settings
	setDifficulty(_gd._diffLevel);
	setMode(_gd._mode);	//calls prepareBackground();

//	_currPos = 0;
	//set pos to -1 if not editing a high score, ie -1=editing high score
//	_pos = (ST_HIGHEDIT==_gd._state)?_gd._score.isHiScore(_mode, _diff):-1;

    setEditing(ST_HIGHEDIT==_gd._state);
    _pos = _gd._score.isHiScore(_mode, _diff);


//##DEBUG
_pos = 3;
setEditing(true);




	//calc hiscore table element positions

	int yUsed = (BG_LINE_BOT - BG_LINE_TOP) - (int)(_gd._fntClean.height() * 10.5);
	_yyGap = yUsed / 11;

	_xDiffLen = _gd._fntClean.calc_text_length("MEDIUM");	//longest of EASY/MEDIUM/HARD
	_xCharLen = _gd._fntClean.calc_text_length("W");

	_xInitsLen = _gd._fntClean.calc_text_length("WWW");
	_xScoreLen = _gd._fntClean.calc_text_length("00000000");
	_xWordsLen = _gd._fntClean.calc_text_length("0000w");
	_xTimesLen = _gd._fntClean.calc_text_length("000s");
	_maxGap    = _gd._fntClean.calc_text_length("XXXXX");


	_gd._score.setCurrInits(_gd._prev_inits);
	_curr = _gd._score.curr();


    tSharedImage &letters = Resource::image("roundel_letters.png");
	_title.setWordCenterHoriz(std::string("HISCORE"), letters, (BG_LINE_TOP-letters.get()->height())/2, 2);
	_title.startMoveFrom( 0, -(letters.get()->height()*2), 15, 100, 0, ROUNDEL_VEL);
	_titleW.start(3000, 1000);  //jumble wait/delay
	_title.setRoundelsId(1);

    tSharedImage &lettersKbd = Resource::image("roundel_kbd_letters.png");
    int xkbd = (Screen::width() - ((lettersKbd->tileW()+8)*10)) / 2; //center top row of kbd on screen
    int ykbd = 	BG_LINE_TOP + _yyGap + _gd._fntClean.height() + (_yyGap*2);
	_kbd.setKbdLetters(lettersKbd, xkbd, ykbd, 6);

    _kbd.setPressEffect("roundel_kbd_ping.png");

    _kbdTileW = lettersKbd->tileW();
    _kbdTileH = lettersKbd->tileH();


	//set the repeat of the keys required
	input->setRepeat(ppkey::UP, 100, 300);		//button, rate, delay
	input->setRepeat(ppkey::DOWN, 100, 300);
	input->setRepeat(ppkey::LEFT, 100, 300);
	input->setRepeat(ppkey::RIGHT, 100, 300);

	//set arrow controls (scroll positions)
    {
    boost::shared_ptr<Sprite> p(new Sprite(Resource::image("btn_round_scroll_up.png")));
    p->setPos(SCREEN_WIDTH-p->tileW(), BG_LINE_TOP+2);
    p->_sigEvent2.Connect(this, &PlayHigh::ControlEvent);
    Control c(p, CTRLID_SCROLL_UP, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsHigh.add(c);
    }
    {
    boost::shared_ptr<Sprite> p(new Sprite(Resource::image("btn_round_scroll_down.png")));
    p->setPos(SCREEN_WIDTH-p->tileW(), BG_LINE_TOP+2+p->tileH()+6);
    p->_sigEvent2.Connect(this, &PlayHigh::ControlEvent);
    Control c(p, CTRLID_SCROLL_DOWN, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsHigh.add(c);
    }
    {
    boost::shared_ptr<Sprite> p(new Sprite(Resource::image("btn_round_scroll_left.png")));
    p->setPos(SCREEN_WIDTH-(p->tileW()*2)-8, BG_LINE_BOT-p->tileH()-2);
    p->_sigEvent2.Connect(this, &PlayHigh::ControlEvent);
    Control c(p, CTRLID_SCROLL_LEFT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsHigh.add(c);
    }
    {
    boost::shared_ptr<Sprite> p(new Sprite(Resource::image("btn_round_scroll_right.png")));
    p->setPos(SCREEN_WIDTH-(p->tileW())-2, BG_LINE_BOT-p->tileH()-2);
    p->_sigEvent2.Connect(this, &PlayHigh::ControlEvent);
    Control c(p, CTRLID_SCROLL_RIGHT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsHigh.add(c);
    }

    //music on/off icon
    { // round music button placed in top left of scorebar
    boost::shared_ptr<Sprite> p(new Sprite(Resource::image("btn_round_music.png")));
    p->setPos(5,5);
    IAudio &audio = Locator::audio();
    Control c(p, CTRLID_MUSIC, 0, Control::CAM_DIS_HIT_IDLE_DOUBLE, audio.musicEnabled()?1:2);
    _controlsHigh.add(c);
    _controlsHigh.enableControl(audio.hasSound(), CTRLID_MUSIC);  //disable override?
    }

    //[EXIT] hi score screen buttons
    {
    boost::shared_ptr<Sprite> p(new Sprite(Resource::image("btn_square_exit_small.png")));
    p->setPos(8, BG_LINE_BOT + ((SCREEN_HEIGHT - BG_LINE_BOT - p->tileH())/2));
    p->_sigEvent2.Connect(this, &PlayHigh::ControlEvent);
    Control c(p, CTRLID_EXIT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsHigh.add(c);
    }
    //[NEXT] for ending inits input
    {
    boost::shared_ptr<Sprite> p(new Sprite(Resource::image("btn_square_next_small.png")));
    int x = (Screen::width() - p->tileW()) / 2; //center
    int y = BG_LINE_BOT - _yyGap - p->tileH();
    p->setPos(x, y);
    p->_sigEvent2.Connect(this, &PlayHigh::ControlEvent);
    Control c(p, CTRLID_NEXT, 0, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsHigh.add(c);
    }

    updateScrollButtons();  //show initial state

	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

void PlayHigh::render(Screen *s)
{
	assert(_init);

	//display:

	//           HIGHSCORES :
	//
	//   ABC ...... 2000000 (#words)
	//   GFG ...... 1232322 (#words)
	//   TTT ......  109999 (#words)
	//   AAA ......    4344 (#words)
	//
	//

	//_menubg->blitTo( s );
	ppg::blit_surface(_menubg->surface(), 0, s->surface(), 0,0);

	//draw screen title roundals
	_title.render(s);

	int yyLine = 0;			        //count lines so far
	int yy = BG_LINE_TOP + _yyGap;	//actual line y pix pos

    if (_mode > GM_REWORD)  //with time column
    {
        const int xUsed = _xDiffLen + _xInitsLen + _xScoreLen + _xWordsLen + _xTimesLen;
        _xxGap = (Screen::width() - xUsed) / 6; //between "DIFF WWW 00000000 0000w 000s"
        if (_xxGap > _maxGap) _xxGap = _maxGap;	//for smaller screens (GP2X)
        const int xDiffTrail = (Screen::width() - xUsed - (_xxGap*6) ) / 2;
        _xxStart = _xxGap + _xDiffLen + xDiffTrail;
    }
    else
    {
        const int xUsed = _xDiffLen + _xInitsLen + _xScoreLen + _xWordsLen;
        _xxGap = (Screen::width() - xUsed) / 5; //between "DIFF WWW 00000000 0000w"
        if (_xxGap > _maxGap) _xxGap = _maxGap;	//for smaller screens (GP2X)
        const int xDiffTrail = (Screen::width() - xUsed - (_xxGap*5) ) / 2;
        _xxStart = _xxGap + _xDiffLen + xDiffTrail;
    }

	_gd._fntClean.put_text(s, _xxGap, yy, _description.c_str(), _diffColour, false);

    int xx(_xxStart);
    if (isEditing())
    {
        _gd._fntClean.put_number(s, xx, yy, _curr.score, "SCORE: %08d", BLACK_COLOUR);
        xx += _xScoreLen*1.5 + _xxGap;
        _gd._fntClean.put_number(s, xx, yy, _curr.words, "WORDS: %04dw", BLACK_COLOUR);
        xx += _xWordsLen*1.5 + _xxGap;
        if (_mode > GM_REWORD)  //speed or timetrial show speed
            _gd._fntClean.put_number(s, xx, yy, _curr.fastest, "SPEED: %03ds", BLACK_COLOUR);

        _kbd.render(s);
    }
    else
    {
        while (yyLine < 10)
        {
            xx = _xxStart;

            const SDL_Color lineColour =
                (yyLine == _pos && _diff == (int)_gd._diffLevel && _mode == (int)_gd._mode)?BLACK_COLOUR:_diffColour;
            //draw scores in order as normal
            _gd._fntClean.put_text(s, xx, yy, _gd._score.inits(_mode, _diff, yyLine).c_str(), lineColour);
            xx += _xInitsLen + _xxGap;
            _gd._fntClean.put_number(s, xx, yy, _gd._score.score(_mode, _diff, yyLine), "%08d", lineColour);
            xx += _xScoreLen + _xxGap;
            _gd._fntClean.put_number(s, xx, yy, _gd._score.words(_mode, _diff, yyLine), "%04dw", lineColour);
            xx += _xWordsLen + _xxGap;
            if (_mode > GM_REWORD)  //speed or timetrial show speed
                _gd._fntClean.put_number(s, xx, yy, _gd._score.fastest(_mode, _diff, yyLine), "%03ds", lineColour);

            ++yyLine;
            yy += _gd._fntClean.height() + _yyGap;
        }
    }

	int helpYpos = BG_LINE_BOT+((SCREEN_HEIGHT-BG_LINE_BOT-_gd._fntClean.height())/2);
	if (!isEditing())
		_gd._fntClean.put_text(s, helpYpos, "Up/down:mode, left/right:difficulty, or EXIT (B)", GREY_COLOUR, true);
	else
		_gd._fntClean.put_text(s, helpYpos, "Enter initials then B to save, or EXIT", GREY_COLOUR, true);

	_controlsHigh.render(s);
}

void PlayHigh::work(Input *input, float speedFactor)
{
	_title.work(input, speedFactor);

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

    if (input->repeat(ppkey::UP))	button(input, ppkey::UP);
    if (input->repeat(ppkey::DOWN)) button(input, ppkey::DOWN);
    if (input->repeat(ppkey::LEFT))	button(input, ppkey::LEFT);
    if (input->repeat(ppkey::RIGHT)) button(input, ppkey::RIGHT);

    _controlsHigh.work(input, speedFactor);

    if (isEditing())
    {
        _kbd.work(input, speedFactor);
    }

}

void PlayHigh::button(Input *input, ppkey::eButtonType b)
{
	switch (b)
	{
	case ppkey::UP:
		if (input->isPressed(b))
			moveUp();
		break;
	case ppkey::DOWN:
		if (input->isPressed(b))
			moveDown();
		break;
	case ppkey::LEFT:
		if (input->isPressed(b))
			moveLeft();
		break;
	case ppkey::RIGHT:
		if (input->isPressed(b))
			moveRight();
		break;
	case ppkey::CLICK:
	case ppkey::B:
		if (input->isPressed(b))
		{
//			if (isEditing() && _currPos < 2)
//			{
//				_currPos++;	//treat as button right unless on 3rd char
//				break;
//			}
//			if (isEditing())	//is editing and position must be 2 (3rd char)
//			{
//				//so player has pressed B on last char to save to entered inits
//				_gd._score.insert(_mode, _diff, _pos, _curr);
//				_gd._score.save();	//save now so player can switch off or return to menu if wishes
//                _gd._prev_inits = _curr.inits;
//
//				setEditing(false);		//set to not-editing
//				break;
//			}

            if (isEditing())
            {
                //user still in play so select curr letter
                if (_kbd.cursorIsTop())
                    _kbd.moveLetterDown();
                else
                {
                    _kbd.moveLetterUp();
                    //if no letters left on bottom row, go back to top
                    if (!_kbd.cursorPrev())
                        _kbd.cursorUp();
                }
            }


			//else pos is -1 (not editing) so follow on to exit...
		}
		//follow on to exit to ST_MENU ...
		//not break
	case ppkey::X:
		if (input->isPressed(b))
		{
			_gd._state = ST_MENU;
			_running = false;	//exit this class running state
		}
		break;
	default:break;
	}
	//update show/hide enable/disable after key press or setEditing(false) etc
	updateScrollButtons();
}

void PlayHigh::moveUp()
{
	//move from C to B to A
	if (isEditing())
	{
		//if (_curr.inits[_currPos] > ' ') --_curr.inits[_currPos];
		_kbd.cursorUp();
	}
	else
		setMode((eGameMode)--_mode);
}

void PlayHigh::moveDown()
{
	//move from A to B to C
	if (isEditing())
	{
		//if (_curr.inits[_currPos] < 'Z') ++_curr.inits[_currPos];
		_kbd.cursorDown();
	}
	else
		setMode((eGameMode)++_mode);
}

void PlayHigh::moveLeft()
{
	if (isEditing())
	{
		//if (_currPos > 0) _currPos--;
		_kbd.cursorPrev();
	}
	else
		setDifficulty((eGameDiff)(_diff-1));
}

void PlayHigh::moveRight()
{
	if (isEditing())
	{
		//if (_currPos < 2) _currPos++;
		_kbd.cursorNext();
	}
	else
		setDifficulty((eGameDiff)(_diff+1));
}

void PlayHigh::updateScrollButtons()
{
    if (isEditing())
    {
        _controlsHigh.showAllControls(false, CTRLID_MUSIC); //show only music
        _controlsHigh.showControl(true, CTRLID_EXIT);       //and exit button (even in edit mode)
    }
    else
    {
        _controlsHigh.showAllControls(true);
        _controlsHigh.enableControl((_mode < GM_MAX-1), CTRLID_SCROLL_UP);
        _controlsHigh.enableControl((_mode > GM_ARCADE), CTRLID_SCROLL_DOWN);
        _controlsHigh.enableControl((_diff > DIF_EASY), CTRLID_SCROLL_LEFT);
        _controlsHigh.enableControl((_diff < DIF_MAX-1), CTRLID_SCROLL_RIGHT);
    }
    //control NEXT moves user inits input editing on to view hiscore proper
    _controlsHigh.showControl(isEditing() && _kbd.getBottomWordLength()==3, CTRLID_NEXT);
}

//event signal from imageanim indicating end of animation
void PlayHigh::ControlEvent(int event, int ctrl_id)
{
    (void)(ctrl_id);    //unused

    if (event == USER_EV_END_ANIMATION)
    {
//        if (ctrl_id == CTRLID_EXIT)  //exit after anim faded
//        {
//            _gd._state = ST_MENU;		//back to menu
//            _running = false;
//            return;
//        }

        updateScrollButtons();
        return;
    }
}

void PlayHigh::handleEvent(SDL_Event &sdlevent)
{
	if (sdlevent.type == SDL_USEREVENT)
	{
		if (USER_EV_END_MOVEMENT == sdlevent.user.code)
		{
            const int rid = reinterpret_cast<int>(sdlevent.user.data2);
		    if (rid == _kbd.getRoundelsId())    //is kbd roundels event, not title jumbling
		    {
                //enables NEXT button if all inits entered, but only after
                //last init stops moving (to bottom row)
                updateScrollButtons();
		    }
		}
	}
}

bool PlayHigh::touch(const Point &pt)
{
    _controlsHigh.touched(pt);    //needed to highlight a touched control

    if (_kbd.touch(pt)) //needed to initiate roundel touch
    {
//        //create a cursor anim here to auto remove on completion
//        SpriteMgr::t_pSprite eff(new Sprite(Resource::image("roundel_kbd_ping.png")));
//
//        Point pt = _kbd.getCurrSelPt();
//        //as cursor(animation) i bigger than kbd tile, calc offset left and up
//        const int x = pt._x - (( eff->tileW() - _kbdTileW) / 2);
//        const int y = pt._y - (( eff->tileH() - _kbdTileH) / 2);
//
//        eff->startAnim( 0, -1, ImageAnim::ANI_ONCEDEL, 25);
//        eff->setPos(x, y);
//        _gd._effects.add(eff);  //add global effect
    }

    return true;
}

//releasing 'touch' press
bool PlayHigh::tap(const Point &pt)
{
    const int ctrl_id = _controlsHigh.tapped(pt);

    if (_kbd.tap(pt))
    {
        //update show/hide enable/disable after key press or setEditing(false) etc
//        updateScrollButtons();
        return true;
    }

    if (ctrl_id == CTRLID_SCROLL_UP)
	{
		moveDown();
        return true;
	}
	else if (ctrl_id == CTRLID_SCROLL_DOWN)
	{
		moveUp();
        return true;
	}
	else if (ctrl_id == CTRLID_SCROLL_LEFT)
	{
		moveLeft();
        return true;
	}
	else if (ctrl_id == CTRLID_SCROLL_RIGHT)
	{
		moveRight();
        return true;
	}
	else if (ctrl_id == CTRLID_EXIT)  //exit after anim faded
    {
        _gd._state = ST_MENU;		//back to menu
        _running = false;
        return true;
    }
    else if (ctrl_id == CTRLID_NEXT)
    {
        _controlsHigh.showControl(false, CTRLID_NEXT); //moves on from editing inits

        //so player has pressed NEXT after last char to save entered inits
        memset(_curr.inits, 0, sizeof(_curr.inits));
        memcpy(_curr.inits, _kbd.getBottomWord().c_str(), std::min(sizeof(_curr.inits), _kbd.getBottomWord().length()));
        _gd._score.insert(_mode, _diff, _pos, _curr);
        _gd._score.save();	//save now so player can switch off or return to menu if wishes
        _gd._prev_inits = _curr.inits;

        setEditing(false);		//set to not-editing
        return true;
    }
    //game music icon action on press, not tap(release)
    if (ctrl_id == CTRLID_MUSIC)// && Locator::audio().musicEnabled())
    {
	    Locator::audio().toggleMusic(true);
        return true;
    }

    return false;
}

//set difficulty locally
void PlayHigh::setDifficulty(eGameDiff diff)
{
	if (diff < DIF_EASY) diff = DIF_EASY;
	if (diff > DIF_HARD) diff = DIF_HARD;
	_diff = diff;
	_diffColour = (DIF_EASY==_diff)?GREEN_COLOUR:(DIF_MED==_diff)?ORANGE_COLOUR:RED_COLOUR;
	setDescription();
}
//set mode locally
void PlayHigh::setMode(eGameMode mode)
{
	if (mode < GM_ARCADE) mode = GM_ARCADE;
	if (mode >= GM_MAX) mode = (eGameMode)((int)GM_MAX - 1);
	_mode = mode;
	setDescription();
	prepareBackground();
}

void PlayHigh::setDescription()
{
	switch (_diff)
	{
	case DIF_EASY:		_description = "EASY";break;
	case DIF_MED:		_description = "MEDIUM";break;
	case DIF_HARD:		_description = "HARD";break;
	default:break;
	}
}

void PlayHigh::prepareBackground()
{
	//create the background to be used for this level,
	//pre drawing so we dont need to do it each frame.
	//...
	_menubg = tSharedImage(new Image());
	//_menubg->cloneFrom(_gd._menubg_plain);	//copy of basic menubg without roundel
	_menubg->cloneFrom(*Resource::image("menubg_plain.png"));	//copy of basic menubg without roundel

    tSharedImage &img = Resource::image("menu_arcade.png");
	int x = - (img->width() /6);	//slightly off screen
	int y = ((SCREEN_HEIGHT - img->height()) / 2) + 2;	//center (+2 for gp2x too high)

	switch (_mode)
	{
	case GM_ARCADE:		//_menubg->blitFrom(&_gd._menu_arcade, -1, x, y);
                        ppg::blit_surface(Resource::image("menu_arcade.png")->surface(), NULL, _menubg->surface(), x, y);
						break;
	case GM_REWORD:		//_menubg->blitFrom(&_gd._menu_reword, -1, x, y);
                        ppg::blit_surface(Resource::image("menu_reword.png")->surface(), NULL, _menubg->surface(), x, y);
						break;
	case GM_SPEEDER:	//_menubg->blitFrom(&_gd._menu_speeder, -1, x, y);
                        ppg::blit_surface(Resource::image("menu_speeder.png")->surface(), NULL, _menubg->surface(), x, y);
						break;
	case GM_TIMETRIAL:	//_menubg->blitFrom(&_gd._menu_timetrial, -1, x, y);
                        ppg::blit_surface(Resource::image("menu_timetrial.png")->surface(), NULL, _menubg->surface(), x, y);
						break;
	default:break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// just a subclass to display and handle a bunch of roundels as a keyboard lyout for hiscore inits
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void RoundelsKbd::setKbdLetters(tSharedImage &letters, int x, int y, int gap)
{


    Roundels::setWord("QWERTYUIOPASDFGHJKLZXCVBNM", letters, x, y, gap, true);
	setRoundelsId(2);

    int xinits = (Screen::width() - ((letters->tileW()+8)*3)) / 2; //center inits row
    setBottomPos(xinits, y + ((letters->tileW()+gap)*3 + letters->tileW()));

    setBottomMax(3);        //3 letter initials for high score
    setBottomCopy(true);    //leaves top word in tact (copy not move tile)

    //now reset the letter positions - 3 rows

	tRoundVect::iterator it = _top.begin();
	int tile(0), column(0);
	for (; it != _top.end(); ++it, ++tile)
	{
        if (!*it) continue;

        const int xPos = x + (column*((*it)->_spr->tileW()+gap));
        (*it)->_spr->setPos(xPos, y);

        column++;
        if (tile == 9 || tile == 18)
        {
            x += ((*it)->_spr->tileW() / 2);    //move each/next row in a bit
            y += (*it)->_spr->tileH()+gap;      //move down with a gap
            column = 0;
        }
	}
}


