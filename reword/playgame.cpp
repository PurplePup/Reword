////////////////////////////////////////////////////////////////////
/*

File:			playgame.cpp

Class impl:		PlayGame

Description:	A class derived from the IPlay interface to handle all screen
				events and drawing of the main game screen. This is the main
				class for the actual playing screen.

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3.1	03.06.2007	Change Stick+L+R exit to allow player to set hiscore if needed
									Fix doDictionary, check if in PG_END
				0.4		13.06.07	Added _dictLine offset into onscreen dict display to scroll lines
										also added scroll arrows to indicate more text to see
									Added popup menu for quick exit/save/move-on
									Changed to stack for various major states in-play and added
										separate functions for each render/work state.
									Added pinger sound when in popup menu, to warn user
				0.5		28.05.08	Added touchscreen support
				0.5.1	02.10.08	Add Pandora and other device screen layout/sizes
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
#include "playgame.h"
#include "utils.h"
#include "helpers.h"
#include "audio.h"
#include "platform.h"
#include "screen.h"
#include "locator.h"
#include "utils.h"

#include "playgamedict.h"

#include <cassert>
#include <algorithm>

//#include <SDL_gfxPrimitives.h>

static	int _countdown;				//seconds remaining

//extern static RandInt g_randInt;


//function called when timer reaches interval set
Uint32 countdown_callback(Uint32 interval, void *param)
{
    (void)(param);

	if (_countdown <= 0)
	{
		ppg::pushSDL_Event(USER_EV_END_COUNTDOWN); //pushEndOfLevel();
		return 0;
	}

	ppg::pushSDL_Event(USER_EV_PING_COUNTDOWN);

	return(interval);
}

//define static
Uint32 PlayGame::next_time = 0;

PlayGame::PlayGame(GameData& gd) :
    _gd(gd), _pPopup(nullptr), _play(nullptr)
{
#ifdef _DEBUG
	_dbg_display = false;
    _debugTotalLetters = _debugNeededAll = _debugNeededNow = 0;
#endif
	_inputL  = _inputR = false;	//keys pressed + stick click (GP2X) to exit game
	_running = false;	//not init yet
	_init = false;		//ditto
	_bAbort = false;	//until L+R+CLICK which exits game to hiscore if needed or to menu
	_bSaveExit = false; //if user saves and exit for later restart
	_tmpDefMore = false;
    _maxwordlen = 0;

    _longestWordLen = 0;
    _shortestWordLen = 0;
    _xxWordHi =  _yyWordHi = 0;
    _boxOffsetY = 0;
    memset(_boxOffset, 0, sizeof(_boxOffset));
    memset(_boxLength, 0, sizeof(_boxLength));
    memset(_boxWordNeeded, 0, sizeof(_boxWordNeeded));
    memset(_boxWordOffset, 0, sizeof(_boxWordOffset));

    _endWorDefExit = "Press " + Locator::input().keyDescription(ppkey::B) + " to exit";
    _endWorDefExitMore = "Click word or " + Locator::input().keyDescription(ppkey::Y) + " for detail, or " + Locator::input().keyDescription(ppkey::B) + " to exit";
    _endWorDefNext = "Press " + Locator::input().keyDescription(ppkey::B) + " to continue";
    _endWorDefNextMore = "Click word or " + Locator::input().keyDescription(ppkey::Y) + " for detail, or " + Locator::input().keyDescription(ppkey::B) + " to continue";

    _nWordBoxHighlightOffset = _nWordBoxEmptyOffset = _nWordBoxNeededOffset = 0;
	_xScratch = _yScratchTop = _yScratchBot = 0;
   	_posRButtonLeft = _posRButtonRight = _posRButtonTop = _posRButtonBot = 0;
	_score0_x = _words0_x = _countdown0_x = 0;
	_score0_y = _words0_y = _countdown0_y = 0;

	_countdownID = 0;

	_success = SU_NONE;
	_bonusScore = _fastest = _fastestCountStart = 0;
	_randomTitle = 0;
    _debugTotalLetters = _debugNeededAll = _debugNeededNow = 0;

    _countdown = 0;     //init static var

	statePush(PG_PLAY);		//also sets default to PG_PLAY
}

PlayGame::~PlayGame()
{
    stopCountdown();
    delete _pPopup;
    delete _play;
}

void PlayGame::init(Input *input, Screen * scr)
{
	//fade out any menu music (but only if no game music still playing)
	//Game music handled separately froim in-game music (mp3 dir etc ?)
    if (Locator::audio().isPlayingMusic()==false)   //no user music
		Mix_FadeOutMusic(3000);                     //fade out any menu music

	_scorebar.load("scorebar.png");
	_cursor.setImage(Resource::image("cursors.png"));
	_scratch.setImage(Resource::image("scratch.png"));
	_boxes.setImage(Resource::image("boxes.png"));


    { // round music button placed in top left of scorebar
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_music.png")));
    IAudio &audio = Locator::audio();
    Control c(p, CTRLID_MUSIC, CTRLGRP_BUTTONS, Control::CAM_DIS_HIT_IDLE_DOUBLE, audio.musicEnabled()?1:2);
    _controlsPlay.add(c);
    _controlsPlay.enableControl(audio.hasSound(), CTRLID_MUSIC);  //disable override?
    }
    { // round fx button placed in top left of scorebar
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_fx.png")));
    IAudio &audio = Locator::audio();
    Control c(p, CTRLID_SFX, CTRLGRP_BUTTONS, Control::CAM_DIS_HIT_IDLE_DOUBLE, audio.sfxEnabled()?1:2);
    _controlsPlay.add(c);
    _controlsPlay.enableControl(audio.hasSound(), CTRLID_SFX);  //disable override?
    }
    { // round up/down buttons used next to word boxes at end level
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_scroll_up_small.png")));
    Control c(p, CTRLID_SCROLL_UP, CTRLGRP_BUTTONS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);
    }
    { // round up/down buttons used next to word boxes at end level
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_scroll_down_small.png")));
    Control c(p, CTRLID_SCROLL_DOWN, CTRLGRP_BUTTONS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);
    }
    { //[MENU] shows in top left (unless [EXIT] shown)
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_menu.png")));
    p->setPos(3, 0);
    Control c(p, CTRLID_MENU, CTRLGRP_BUTTONS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);
    }
    { //[EXIT] goes in same place as [MENU] when game over
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_exit.png")));
    p->setPos(3, 0);
    p->setVisible(false);  //not available until countdown finished
    Control c(p, CTRLID_EXIT, CTRLGRP_BUTTONS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);
    }
    { //[NEXT] goes over the countdown position
    t_pSharedSpr p(new Sprite(Resource::image("btn_square_next.png")));
    p->setPos(Screen::width() - p->tileW() - 3, 0);
    p->setVisible(false);  //not available until countdown finished
    Control c(p, CTRLID_NEXT, CTRLGRP_BUTTONS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);

    //bounding rect of [NEXT] button placed over countdown timer
    //doesn't move so can be cached here
    _pause_rect = p->bounds();
    }

    //now the four letter/word controls... positions set in newLevel()
    { //shuffle
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_word_shuffle.png")));
    Control c(p, CTRLID_SHUFFLE, CTRLGRP_LETTERS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);
    }
    { //try word
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_word_try.png")));
    Control c(p, CTRLID_TRYWORD, CTRLGRP_LETTERS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);
    }
    { //totop
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_word_totop.png")));
    Control c(p, CTRLID_TOTOP, CTRLGRP_LETTERS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);
    }
    { //last
    t_pSharedSpr p(new Sprite(Resource::image("btn_round_word_last.png")));
    Control c(p, CTRLID_LAST, CTRLGRP_LETTERS, Control::CAM_DIS_HIT_IDLE_SINGLE);
    _controlsPlay.add(c);
    }

    _nWordBoxHighlightOffset = (_boxes.tileCount()/4); //4 blocks of word boxes
    _nWordBoxEmptyOffset = ((_boxes.tileCount()/4)*2); //4 blocks.. 3rd block
    _nWordBoxNeededOffset = ((_boxes.tileCount()/4)*3); //4 blocks.. 4th block

	//set the repeat of the keys required
	input->setRepeat(ppkey::UP, 250, 250);		//button, rate, delay
	input->setRepeat(ppkey::DOWN, 250, 250);
	input->setRepeat(ppkey::LEFT, 250, 250);
	input->setRepeat(ppkey::RIGHT, 250, 250);

	//once the class is initialised, init and running are set true
	newGame();	//reset scores etc
	if (!newLevel())	//preparebackground, get next word etc
    {
        //already called exit(ST_MENU);
        return;
    }

    _round.setPressEffect("ping_small.png");

	//need to set the _init and _running flags
	_init = true;
	_running = true;
}

//exit the game play screen with a specific state, eg. back to ST_MENU
void PlayGame::exit(eGameState toState)
{
	stopCountdown();
	clearEventBuffer();
	_gd._state = toState;	//ST_MENU, ST_PLAY etc
//	std::cout << "state: exit = " << toState << std::endl;
	_running = false;
}

//do not call directly. Use statePush() & statePop()
void PlayGame::stateFn(eState state)
{
    //these state changes could be done better as actual classes (as PG_DICT) is now done
    //but as most are small states, they haven't been converted to their own classes yet.

    if (_play)
    {
        delete _play;
        _play = nullptr;
    }

	switch (state)
	{
	//note PG_PLAY: is default:

	case PG_WAIT:	pRenderFn = &PlayGame::render_wait;
					pWorkFn = &PlayGame::work_wait;
					pButtonFn = &PlayGame::button_wait;
					pTouchFn = &PlayGame::touch_default;
					break;	//same renderer as current

	case PG_END:	pRenderFn = &PlayGame::render_end;
					pWorkFn = &PlayGame::work_end;
					pButtonFn = &PlayGame::button_end;
					pTouchFn = &PlayGame::touch_end;
                    //some setup required
					state_end_setup_scrollers();
					break;

    case PG_DICT:   {//new play class overrides currently mapped state functions - see render() etc
                    _play = new PlayGameDict(_gd, _dictWord);
                    Input &i = static_cast<Input&>(Locator::input());
                    _play->init(&i);
                    }
                    break;

	case PG_PAUSE:	pRenderFn = &PlayGame::render_pause;
					pWorkFn = &PlayGame::work_pause;
					pButtonFn = &PlayGame::button_pause;
					pTouchFn = &PlayGame::touch_pause;
					break;
//	case PG_PAUSE:	_play = new PlayGamePause(_gd, _dictWord);  //TODO...
//					break;

	case PG_PLAY:
	default:		pRenderFn = &PlayGame::render_play;
					pWorkFn = &PlayGame::work_play;
					pButtonFn = &PlayGame::button_play;
					pTouchFn = &PlayGame::touch_play;
					break;

	}
}
void PlayGame::statePush(eState state)
{
	_state = state;	//for quick reference
	_states.push(state);
	stateFn(state);
}
PlayGame::eState PlayGame::statePop()
{
	eState state = _states.pop();
	stateFn(state);
	return _state = state;
}

//main render fn called by Game class
void PlayGame::render(Screen* s)
{
    if (_play)
    {
        //dictionary screen etc
        _play->render(s);
        return;
    }

	//depending on state, call the function pointer of the
	//currently mapped render function
	(*this.*pRenderFn)(s);

	//render popup menu on top of curr screen
	if (_pPopup)
        render_popup(s);

}

//display the game screen PG_PLAY
void PlayGame::render_play(Screen* s)
{
	int xx, yy;

	//draw background
	s->blit(_gamebg->texture(), nullptr, 0, 0);

 	//draw scores and coloured seconds countdown
	_gd._fntClean.put_text(s, _score_x, _score0_y, "SCORE:", BLACK_COLOUR);
	_gd._fntClean.put_number(s, _score0_x, _score0_y, _gd._score.currScore(), "%08d", GREEN_COLOUR);
	_gd._fntClean.put_text(s, _words_x, _score0_y, "WORDS:", BLACK_COLOUR);
	_gd._fntClean.put_number(s, _words0_x, _words0_y, _gd._score.currWords(), "%04d", BLUE_COLOUR);
	//(>10) normal countdown in "plenty of time" colour
	//(<=10) countdown in "oh crap" colour (red to denote time running out)
	//		A warning "ping" is also sounded in the countdown callback fn each second...


	if (PG_PLAY == _state)
	{
        //center the countdown text in the scorebar countdown [area]
        _gd._fntBig.put_number_mid(s, _countdown0_x, _countdown0_y, //s->surface()->w - 54,
            _countdown, "%d", (_countdown > 10)?YELLOW_COLOUR:RED_COLOUR, true);	//TIME:

		//draw the tile background before any lower letters placed
	    //expecting a scratch image list of: "[>][3][4][5][6][7][8]"
	    //resolve to show: [>][>][3][4][5][6]   to   [>][>][>][5][6][7][8]
	    const int nArrows = _shortestWordLen-1;   //num of [arrows] before [numbers]
		for (xx=0; xx<_longestWordLen; xx++)
		{
		    //if no words of a certain length, place a [>], else place [N]
            int nTile(0);   //point to [>] tile
            if (xx >= nArrows && _gd._words.wordsOfLength(xx+1) > 0) nTile = xx-1;   //point to [N] tile
			_scratch.blitTo(s, _xScratch+(xx*(CURSORW+2)), _yScratchBot, nTile);
		}

		//draw touch controls not in controls container
 //       _gamemusic_icon.draw(s);

		//draw game letters - top and bottom
		_round.render(s);

		_cursor.blitTo(s, _xScratch+(_round.currentX()*(CURSORW+2)),
						(_round.cursorIsTop()?_yScratchTop:_yScratchBot),
                        //(int)_gd._diffLevel-1     //curr diff colour
                        _gd._words.getWordLevel()-1 //colour of word (always <= curr diff)
                     );
	}

	//draw word boxes 1 length at a time downwards (easier)
	const int yo = _yScratchBot + CURSORH + _boxOffsetY;	//start y offset
	tWordsFoundList::const_iterator it;

    const bool bHighlightAllColumns = PG_PLAY == _state && foundEnoughWords();

	//[3..], [4...], [5....], [N.....] letter boxes
	for (xx=_shortestWordLen; xx<=_longestWordLen; ++xx)	//accross the screen
	{
	    const int yyWordOff = _boxWordOffset[xx];
		it = _wordsFound[xx].begin()+yyWordOff;
		const int nWords = _gd._words.wordsOfLength(xx);
		const bool bHighlightWholeColumn = bHighlightAllColumns || (PG_PLAY == _state && _round.getBottomWordLength() == xx);

		if (nWords)	//column has words
		{
		    yy = 0;
			for (int yyCur = yyWordOff; yyCur<nWords; ++yyCur)	//down the screen
			{
			    const int boxOffsetY = yo+(yy*(BOXH + BOXHGAP));  //same as in touch_end()

				//A gap exists below the last word box in the column whch can display the number of
				//remaining words that can be found but not displayable in the list.
				if (yy >= MAX_WORD_COL)
				{
					if (PG_PLAY == _state)
						_gd._fntTiny.put_number(s, _boxOffset[xx]+6, boxOffsetY,
                              (nWords-MAX_WORD_COL) - (_wordsFound[xx].size() > MAX_WORD_COL ? _wordsFound[xx].size() - MAX_WORD_COL : 0),
                            "%d more",
							(nWords==(int)_wordsFound[xx].size())?GREEN_COLOUR:BLUE_COLOUR);

					break;	//out of this n letters loop
				}

				if (it != _wordsFound[xx].end())
				{
					//display short dictionary def
					if (PG_END == _state && xx == _xxWordHi && yy == _yyWordHi)
					{
						//level complete, all, some or no words found, check for dictionary
						int yd = (_yScratchBot+CURSORH+_boxOffsetY) - (2 * _gd._fntClean.height()) - (_gd._fntClean.height()/2);	//put comment just above found words in boxes
						_tmpStr2 = (*it)._word;
						_tmpDefStr = _gd._words.getDictForWord(_tmpStr2)._description;
                        _tmpDefMore = false;
						if (_tmpDefStr.length())
						{
							if (_tmpDefStr.length() > FONT_CLEAN_MAX)	//make "definition..."
							{
								_tmpDefStr.erase(FONT_CLEAN_MAX-3);	//no more than NN-3 chars
								_tmpDefStr += "...";	//add ellipsis to make NN again
								_tmpDefMore = true;
							}
							//display the word description
							_gd._fntClean.put_text(s, yd, _tmpDefStr.c_str(), BLACK_COLOUR);
						}
						else	//blank word - no dictionary entry
							_gd._fntClean.put_text(s, yd, "This word has no definition", GREY_COLOUR);
					}

                    //word for this box could be normal background or in Arcade mode, could be a highlight (green or yellow)
                    _boxes.blitTo( s, _boxOffset[xx], boxOffsetY,					//tile 0=3, 1=4, 2=5, 3=6 etc. letter words
                        (
                            ((PG_PLAY != _state && xx == _xxWordHi && yy == _yyWordHi) || bHighlightWholeColumn) ?
                                xx+_nWordBoxHighlightOffset :  //xx+(count/4) for n box blocks in boxes.png
                                (PG_PLAY == _state && _maxwordlen!=_longestWordLen && yy < _boxWordNeeded[xx]) ?
                                    xx+_nWordBoxNeededOffset :      //yellow
                                    xx //+_nWordBoxHighlightOffset     //green
                        )-3 );  //-3 to reset the xx back to 0 as the boxes.png starts at 3 letter tile

					//Display the word in red (not found) or blue (found)
					//Only found words populate the container during play, so red only drawn at end of level
					_gd._fntClean.put_text_mid(s, _boxOffset[xx]+(_boxLength[xx]/2), 
							boxOffsetY + ((BOXH - _gd._fntClean.height()) /2),
							(*it)._word.c_str(), ((*it)._found)?BLUE_COLOUR:RED_COLOUR);

					++it;   //next found word
				}
				else
                {
                    //no word for this box so just empty (but possibly highlight for Arcade 'to be found' box)
                    //or show in red if end of level (not in-play)
                    _boxes.blitTo( s, _boxOffset[xx], boxOffsetY,					//tile 0=3, 1=4, 2=5, 3=6 etc. letter words
                        (
                            ((PG_PLAY != _state && xx == _xxWordHi && yy == _yyWordHi) || bHighlightWholeColumn) ?
                                xx+_nWordBoxHighlightOffset :  //xx+(count/4) for n box blocks in boxes.png
                                (PG_PLAY == _state && _maxwordlen!=_longestWordLen && yy < _boxWordNeeded[xx]) ?
                                    xx+_nWordBoxNeededOffset :
                                    xx
                        )-3 );  //-3 to reset the xx back to 0 as the boxes.png starts at 3 letter tile
                }

                ++yy;
			}
		}
		else
		{
		    //draw empty rect to denote no words of xx length at all
            //using sdlGfxPrimitives: rectangleRGBA(s->surface(), _boxOffset[xx], yo, _boxOffset[xx]+_boxLength[xx], yo+BOXH, 100, 100, 100, 100); //grey, faded
            if (_gd._mode <= GM_REWORD) //only ARCADE and REWORD modes have empty boxes drawn
                _boxes.blitTo( s, _boxOffset[xx], yo, xx+(_nWordBoxEmptyOffset-3));	//tile 0=3, 1=4, 2=5, 3=6 etc. letter words
		}
	}	//for

	//play controls always sit on top of anything else
    _controlsPlay.render(s);

#if defined(_DEBUG)
    if (_dbg_display)
    {

        _gd._fntClean.put_text(s, 0, SCREEN_HEIGHT - _gd._fntClean.height(), _gd._words.getWordTarget().c_str(), BLUE_COLOUR);	//##DEBUG## shows first target word

        _gd._fntClean.put_number(s, 0, SCREEN_HEIGHT-88, _debugTotalLetters, "%d", BLUE_COLOUR);
        _gd._fntClean.put_number(s, 0, SCREEN_HEIGHT-66, _debugNeededAll, "%d", BLUE_COLOUR);
        _gd._fntClean.put_number(s, 0, SCREEN_HEIGHT-44, _debugNeededNow, "%d", BLUE_COLOUR);
    }
#endif
}

void PlayGame::render_wait(Screen* s)
{
	//##TODO - call some animation or something here before
	//			going to PG_END and user can move on to next level
	render_end(s);

/*
	render_play(s);	//same as PG_PLAY... just waiting
*/
}

void PlayGame::render_end(Screen* s)
{
	//first call main game render as our popup just overlays
	render_play(s);

	//finished level so show success type and bonus etc
	const int minGap = _gd._fntSmall.height();	//useful distance based on small font
	const int yyTitle = _scorebar.height() + minGap;
	const int yyReward = yyTitle +_gd._fntBig.height() + minGap;
	const int yyBonus = (_yScratchBot+CURSORH+_boxOffsetY) + (2*_gd._fntBig.height());
	switch (_success)
	{
	case SU_ARCADE:		//WELL DONE! - You got enough
		switch (_randomTitle)
		{
		case 0:_gd._fntBig.put_text(s, yyTitle, "ALRIGHT!", PURPLE_COLOUR, true);break;
		case 1:_gd._fntBig.put_text(s, yyTitle, "WOOHOO!", PURPLE_COLOUR, true);break;
		case 2:_gd._fntBig.put_text(s, yyTitle, "STORMING!", PURPLE_COLOUR, true);break;
		default:_gd._fntBig.put_text(s, yyTitle, "WELL DONE!", PURPLE_COLOUR, true);break;
		}
		_gd._fntMed.put_number(s, yyReward, _bonusScore, "Just enough, add %d points", PURPLE_COLOUR, false);
		break;
	case SU_GOT6:		//WELL DONE! - You got the 6
		switch (_randomTitle)
		{
		case 0:_gd._fntBig.put_text(s, yyTitle, "GREAT!", PURPLE_COLOUR, true);break;
		case 1:_gd._fntBig.put_text(s, yyTitle, "GOOD!", PURPLE_COLOUR, true);break;
		case 2:_gd._fntBig.put_text(s, yyTitle, "COOL!", PURPLE_COLOUR, true);break;
		default:_gd._fntBig.put_text(s, yyTitle, "WELL DONE!", PURPLE_COLOUR, true);break;
		}
		_gd._fntMed.put_number(s, yyReward, _bonusScore, "You got a Re-word, add %d points", PURPLE_COLOUR, false);
		break;
	case SU_BONUS:		//BONUS!! - You got all words
		switch (_randomTitle)
		{
		case 0:_gd._fntBig.put_text(s, yyTitle, "FABULOUS!", PURPLE_COLOUR, true);break;
		case 1:_gd._fntBig.put_text(s, yyTitle, "EXCELLENT!", PURPLE_COLOUR, true);break;
		case 2:_gd._fntBig.put_text(s, yyTitle, "AMAZING!!", PURPLE_COLOUR, true);break;
		default:_gd._fntBig.put_text(s, yyTitle, "AWESOME!!", PURPLE_COLOUR, true);break;
		}
		_gd._fntMed.put_number(s, yyReward, _bonusScore, "All words, add %d points", PURPLE_COLOUR, false);
		break;
	case SU_BADLUCK:	//Bad Luck! - Out of time
		switch (_randomTitle)
		{
		case 0:_gd._fntBig.put_text(s, yyTitle, "BUMMER!", RED_COLOUR, true);break;
		case 1:_gd._fntBig.put_text(s, yyTitle, "WHAT A SHAME!", RED_COLOUR, true);break;
		case 2:_gd._fntBig.put_text(s, yyTitle, "OH NO!!", RED_COLOUR, true);break;
		default:_gd._fntBig.put_text(s, yyTitle, "BAD LUCK!", RED_COLOUR, true);break;
		}
		_gd._fntMed.put_text(s, yyReward, "Out of time", RED_COLOUR, false);
		break;

	case SU_SPEEDER:		//Speeder mode - a single 6 letter word achieved
		if (_fastest)
		{
			_gd._fntBig.put_text(s, yyTitle, "FASTEST YET!", GOLD_COLOUR, true);
			_gd._fntMed.put_number(s, yyReward, _fastest, "You got it in %d seconds!", GREEN_COLOUR, false);
			_gd._fntMed.put_number(s, yyBonus, (maxCountdown()-_fastest)*SCORE_FASTEST, "Bonus: %d !", GREEN_COLOUR, false);
		}
		else
		{
			switch (_randomTitle)
			{
			case 0:_gd._fntBig.put_text(s, yyTitle, "GREAT!", GOLD_COLOUR, true);break;
			case 1:_gd._fntBig.put_text(s, yyTitle, "EXCELLENT!", GOLD_COLOUR, true);break;
			case 2:_gd._fntBig.put_text(s, yyTitle, "GOOD!", GOLD_COLOUR, true);break;
			default:_gd._fntBig.put_text(s, yyTitle, "WELL DONE!", GOLD_COLOUR, true);break;
			}
			_gd._fntMed.put_text(s, yyReward, "Now try another", GREEN_COLOUR, false);
		}
		break;
	case SU_TIMETRIAL:	//Time trial - single count  to 0 - no timer reset
		if (_fastest)
		{
			_gd._fntBig.put_text(s, yyTitle, "FASTEST YET!", BLUE_COLOUR, true);
			_gd._fntMed.put_number(s, yyReward, _fastest, "You got it in %d seconds!", GREEN_COLOUR, false);
			_gd._fntMed.put_number(s, yyBonus, (maxCountdown()-_fastest)*SCORE_FASTEST, "Bonus: %d !", GREEN_COLOUR, false);
		}
		else
		{
			switch (_randomTitle)
			{
			case 0:_gd._fntBig.put_text(s, yyTitle, "QUICKLY!", BLUE_COLOUR, true);break;
			case 1:_gd._fntBig.put_text(s, yyTitle, "HURRY!", BLUE_COLOUR, true);break;
			case 2:_gd._fntBig.put_text(s, yyTitle, "GREAT!", BLUE_COLOUR, true);break;
			default:_gd._fntBig.put_text(s, yyTitle, "WELL DONE!", BLUE_COLOUR, true);break;
			}
			_gd._fntMed.put_text(s, yyReward, "get another one...", GREEN_COLOUR, false);
		}
		break;
    case SU_SAVEEXIT:
		_gd._fntBig.put_text(s, yyTitle, "Game Saved", GREEN_COLOUR, true);
		_gd._fntSmall.put_text(s, yyReward, "Continue this game later!", BLUE_COLOUR, false);
		break;

	case SU_GAMEOVER:	//Game Over - You're a high scorer!
	default:
		_gd._fntBig.put_text(s, yyTitle, "Game Over", RED_COLOUR, true);
        if (_gd._score.isHiScore(_gd._mode, _gd._diffLevel) != -1)
            _gd._fntMed.put_text(s, yyReward, "But you're a high scorer!", GREEN_COLOUR, false);
        else
            _gd._fntMed.put_text(s, yyReward, "Better luck next time", BLUE_COLOUR, false);
		break;
	}

	//helpful message - if a dict definition available tell player
	if (PG_END == _state)	//BODGE - only show if in end state(as render_wait() calls this), otherwise "Press B..." shows for a second
	{
        //put comment just above found words in boxes
		const int yo = (_yScratchBot+CURSORH+_boxOffsetY) - _gd._fntClean.height() - (_gd._fntClean.height()/2);
		_gd._fntClean.put_text(s, yo,
            (_success == SU_GAMEOVER || _success == SU_SAVEEXIT) ?
                ((_tmpDefMore) ? _endWorDefExitMore.c_str() : _endWorDefExit.c_str()) :
                ((_tmpDefMore) ? _endWorDefNextMore.c_str() : _endWorDefNext.c_str()),
            GREY_COLOUR, false);
	}
}

void PlayGame::render_pause(Screen* s)
{
	const SDL_Colour c = BLACK_COLOUR;
	//ppg::drawSolidRect(s->surface(), 0,0,s->width(), s->height(), c);
	s->drawSolidRect( 0, 0, s->width(), s->height(), c);
	_roundPaused->render(s);
    _gd._fntClean.put_text(s, (SCREEN_HEIGHT/2) + 30,
                        "10 second penalty", WHITE_COLOUR, false);
	return;
}

//render fn used by PG_POPUP
void PlayGame::render_popup(Screen* s)
{
	//this fn called after any other 'curr' screen so it floats "above"
	_pPopup->render(s);
	//show play controls during popup too
    _controlsPlay.render(s);
	return;
}

void PlayGame::work(Input* input, float speedFactor)
{
    if (_play)
    {
        //dictionary screen etc
        _play->work(input, speedFactor);
        return;
    }
    //depending on state, call the function pointer of the
	//correctly mapped work function

	(*this.*pWorkFn)(input, speedFactor);

	//handle popup menu on top of curr screen
	if (_pPopup)
        work_popup(input, speedFactor);

    _controlsPlay.work(input, speedFactor);
}

void PlayGame::work_play(Input* input, float speedFactor)
{
    (void)(speedFactor);
	//Do repeat keys...
	//if a key is pressed and the interval has expired, process
	//that button as if pressesd again

    if (input->repeat(ppkey::UP))	button(input, ppkey::UP);
    if (input->repeat(ppkey::DOWN))  button(input, ppkey::DOWN);
    if (input->repeat(ppkey::LEFT))	button(input, ppkey::LEFT);
    if (input->repeat(ppkey::RIGHT)) button(input, ppkey::RIGHT);

	_round.work(input, speedFactor);

//  audio not animated icons so don't need to call work()
//	_gd._gamemusic_icon.work();

}

void PlayGame::work_wait(Input* input, float speedFactor)
{
    (void)(input);
    (void)(speedFactor);
	//wait.... a while
	if (_waiting.done())
	{
		//_state = PG_END;	//move into end of level mode
		statePop();	//out of waiting state
		statePush(PG_END);	//into end state
	}
}

void PlayGame::work_end(Input* input, float speedFactor)
{
    (void)(input);
    (void)(speedFactor);

    if (input->repeat(ppkey::UP))	button(input, ppkey::UP);
    if (input->repeat(ppkey::DOWN))  button(input, ppkey::DOWN);
}

void PlayGame::work_pause(Input* input, float speedFactor)
{
    (void)(input);
    (void)(speedFactor);
	_roundPaused->work(input, speedFactor);
}

void PlayGame::work_popup(Input* input, float speedFactor)
{
	_pPopup->work(input, speedFactor);
}

void PlayGame::startPopup(Input *input)
{
    (void)(input);

    slideRoundButtonsOut();

	_pPopup = new PlayGamePopup(_gd, _state!=PG_END, foundEnoughWords());
	if (_pPopup)
		_pPopup->init(input, &Locator::screen());
}

void PlayGame::stopPopup()
{
	delete _pPopup;
	_pPopup = nullptr;

    slideRoundButtonsIn();
}

bool PlayGame::button(Input* input, ppkey::eButtonType b)
{
	//first handle any global keys
	switch (b)
	{
	case ppkey::L:
		_inputL = input->isPressed(b);
		break;
	case ppkey::R:
		_inputR = input->isPressed(b);
		break;
	case ppkey::SELECT:
		if (input->isPressed(b))
		{
            if (_state != PG_PAUSE)
            {
                _controlsPlay.forceFade(CTRLID_MENU);

                if (_pPopup)
                    stopPopup();
                else
                    startPopup(input);
            }
		}
		break;
	case ppkey::CLICK:
		if (input->isPressed(b))
		{
			if (_inputL && _inputR) //GP2X type exit
			{
				_bAbort = true;
				ppg::pushSDL_Event(USER_EV_END_COUNTDOWN); //try to exit more gracefully, pushes end of level
			}
			else
			{
			    Locator::audio().startNextTrack();	//try to start next music track
			}
		}
		break;
	default:
		break;
	}

    if (_play)
    {
        //other 'embedded' screen, eg. dictionary screen etc
        return _play->button(input, b);
    }

	//now handle popup menu on top of curr screen or depending on state,
	//call the function pointer of the correctly mapped button function
	if (_pPopup)
		return button_popup(input, b);
	else
		return (*this.*pButtonFn)(input, b);

}

bool PlayGame::button_play(Input* input, ppkey::eButtonType b)
{
	if (!_waiting.done()) return false;	//no user input until finished waiting

	//not waiting so allow button use...

    //intercept any PC or PANDORA keyboard a-z key presses to process
    //in roundels class - bypassing the normal console controls to
    //move letters down, allowing user to type the word.
    if (_round.button(input, b))
        return true;    //roundel processed the a-z keys

	switch (b)
	{
	case ppkey::LEFT:
		if (input->isPressed(b)) _round.cursorPrev();
		break;
	case ppkey::RIGHT:
		if (input->isPressed(b)) _round.cursorNext();
		break;
	case ppkey::UP:
		if (input->isPressed(b)) _round.cursorUp();
		break;
	case ppkey::DOWN:
		if (input->isPressed(b)) _round.cursorDown(); //will only go down if letters exist
		break;
	case ppkey::L:  //left shoulder
	case ppkey::R:  //right shoulder
		if (input->isPressed(b))
		{
		    _controlsPlay.forceFade(CTRLID_LAST);
		    commandWordToLast();
		}
		break;
	case ppkey::A:
		if (input->isPressed(b))
		{
		    _controlsPlay.forceFade(CTRLID_SHUFFLE);
			commandJumbleWord();
		}
		break;
	case ppkey::B:
		if (input->isPressed(b))
		{
			//user still in play so select curr letter
			if (_round.cursorIsTop())
			{
	  	 		_round.moveLetterDown();
			}
			else
			{
	    		_round.moveLetterUp();
				//if no letters left on bottom row, go back to top
				if (!_round.cursorPrev()) _round.cursorUp();
			}
//			_round.cursorNext();	//dont use as it confuses player (well me anyway)
		}
		break;
	case ppkey::START:
		if (input->isPressed(b)) doPauseGame();
		break;
	case ppkey::Y:
		if (input->isPressed(b))
		{
		    _controlsPlay.forceFade(CTRLID_TOTOP);
		    commandClearAllToTop();
		}
		break;
	case ppkey::X:
		if (input->isPressed(b))
		{
		    _controlsPlay.forceFade(CTRLID_TRYWORD);
		    commandTryWord();
		}
		break;

	default:
        return false;
	}
	return true;
}
bool PlayGame::button_wait(Input* input, ppkey::eButtonType b)
{
	//currently same as play state
	return button_play(input, b);
}
bool PlayGame::button_end(Input* input, ppkey::eButtonType b)
{
	//in end state, not waiting so allow button use...
	switch (b)
	{
	case ppkey::X:
	case ppkey::B:
		if (input->isPressed(b))
		{
			doMoveOn();

		    //would need to wait for forced fade to end before screen exit to see any effect
		    //on screen, so don't bother at the moment.
		    //_controlsPlay.forceFade(_running?CTRLID_NEXT:CTRLID_EXIT);
		}
		break;
	case ppkey::LEFT:
		if (input->isPressed(b))
		{
			do {	//repeat jump left if finds a gap (missing 3, 4, 5 letter word)
				if (_xxWordHi-1 < _shortestWordLen) _xxWordHi = _longestWordLen; else --_xxWordHi;
			} while (_gd._words.wordsOfLength(_xxWordHi) == 0);
			if (_yyWordHi > _gd._words.wordsOfLength(_xxWordHi)-1) _yyWordHi = _gd._words.wordsOfLength(_xxWordHi)-1;
		}
		break;
	case ppkey::RIGHT:
		if (input->isPressed(b))
		{
			do {	//repeat jump right if finds a gap (missing 3, 4, 5 letter word)
				if (_xxWordHi+1 > _longestWordLen) _xxWordHi = _shortestWordLen; else ++_xxWordHi;
			} while (_gd._words.wordsOfLength(_xxWordHi) == 0);
			if (_yyWordHi > _gd._words.wordsOfLength(_xxWordHi)-1) _yyWordHi = _gd._words.wordsOfLength(_xxWordHi)-1;
		}
		break;
	case ppkey::UP:
		if (input->isPressed(b))
		{
		    button_end_up();
		}
		break;
	case ppkey::DOWN:
		if (input->isPressed(b))
		{
		    button_end_down();
		}
		break;

	case ppkey::Y:
	case ppkey::CLICK:
//		if (!_bAbort && //make sure global L+R+Click not pressed
		if (input->isPressed(b))
		{
			doDictionary();		//toggle dictionary mode
		}
		break;

	default:return false;
	}

	state_end_setup_scrollers();
	return true;
}

//user pressed up button or the up scroll control
void PlayGame::button_end_up()
{
    int maxOnScreen = std::min(MAX_WORD_COL, _gd._words.wordsOfLength(_xxWordHi));
    if (_yyWordHi <= 0) //first box in col
    {
        int yyWordOff = _boxWordOffset[_xxWordHi];
        if (yyWordOff > 0)
        {
            //sub one from offset and leave highligh at first box
            yyWordOff--;
        }
        else
        {
            //move to end - to bottom of screen col and word list
            yyWordOff = std::max(0, _gd._words.wordsOfLength(_xxWordHi) - maxOnScreen -1); //0 if < MAX_.. boxes
            _yyWordHi = maxOnScreen-1;
        }
        _boxWordOffset[_xxWordHi] = yyWordOff;
    }
    else
    {
        _yyWordHi--;    //just move highlight up
    }
}

//user pressed down button or the down scroll control
void PlayGame::button_end_down()
{
    int maxOnScreen = std::min(MAX_WORD_COL, _gd._words.wordsOfLength(_xxWordHi));
    if (_yyWordHi >= maxOnScreen-1) //last box in col
    {
        int yyWordOff = _boxWordOffset[_xxWordHi];
        if (yyWordOff + maxOnScreen < _gd._words.wordsOfLength(_xxWordHi)-1)
        {
            //add one to offset and leave highlight at last box
            yyWordOff++;
        }
        else
        {
            //back to start - to top
            yyWordOff = _yyWordHi = 0;
        }
        _boxWordOffset[_xxWordHi] = yyWordOff;
    }
    else
    {
        _yyWordHi++;    //just move highlight down
    }
}

void PlayGame::state_end_setup_scrollers()
{
    if (_state == PG_END && _gd._words.wordsOfLength(_xxWordHi)-1 > MAX_WORD_COL)
    {
        const int yo = _yScratchBot + CURSORH + _boxOffsetY;	//start y offset
        Control *p = _controlsPlay.getControl(CTRLID_SCROLL_UP);
        if (p)
        {
            Sprite *s = p->getSprite();
            s->setPos(_boxOffset[_xxWordHi] - (1.5*s->tileW()), yo);
            _controlsPlay.enableControl(true, CTRLID_SCROLL_UP);
        }
        p = _controlsPlay.getControl(CTRLID_SCROLL_DOWN);
        if (p)
        {
		    const int yo2 = yo+((MAX_WORD_COL-1)*(BOXH + BOXHGAP));  //same as in touch_end()
            Sprite *s = p->getSprite();
            s->setPos(_boxOffset[_xxWordHi] - (1.5*s->tileW()), yo2);
            _controlsPlay.enableControl(true, CTRLID_SCROLL_DOWN);
        }
    }
    else
    {
        _controlsPlay.showControl(false, CTRLID_SCROLL_UP);
        _controlsPlay.showControl(false, CTRLID_SCROLL_DOWN);
    }
}

bool PlayGame::button_pause(Input* input, ppkey::eButtonType b)
{
	switch (b)
	{
	case ppkey::START:
	case ppkey::B:
		if (input->isPressed(b)) doPauseGame();	//will un-pause if it's in pause mode
		break;

	default:return false;
	}
	return true;
}
bool PlayGame::button_popup(Input* input, ppkey::eButtonType b)
{
	const bool bRet = _pPopup->button(input, b);
	handlePopup();
	return bRet;
}
void PlayGame::handlePopup()
{
	if (!_pPopup->isSelected()) return;

	switch (_pPopup->selectedId())
	{
	case PlayGamePopup::POP_CANCEL:
		//continue with game level
//		stopPopup();
//		slideRoundButtonsIn();
//		return;
        break;
	case PlayGamePopup::POP_SKIP:
		if (_state != PG_END)	//not already at end
			ppg::pushSDL_Event(USER_EV_END_COUNTDOWN);	//next level - if not got a 6, will end game
		break;
	case PlayGamePopup::POP_SAVE:
		//user selected to save state (now at least a 6 letter word found)
		//and allow resume game later, so follow on to POP_QUIT
		_bSaveExit = true;
	case PlayGamePopup::POP_QUIT:
		//back to main menu - like L+R+Click (or ESC on PC)
		_bAbort = true;
		ppg::pushSDL_Event(USER_EV_END_COUNTDOWN); //try to exit more gracefully, pushes end of level
		break;

	default:
//        stopPopup();
//   		slideRoundButtonsIn();
//        return;	//do nothing
        break;
	}

	stopPopup();	//deletes _pPopup after user selects, if SELECT pressed, button() fn stops popup
}

bool PlayGame::touch(const Point &pt)
{
    if (_play)
    {
        //dictionary screen etc
        return _play->touch(pt);
    }

	if (_pPopup)
	{
        _controlsPlay.touched(pt);    //needed to highlight a touched control
		_pPopup->touch(pt);
		handlePopup();
	}
	else
		(*this.*pTouchFn)(pt);  //call one of the other touch_xxx functions

    return true;
}
bool PlayGame::touch_default(const Point &pt)
{
    (void)(pt);

	return true;
}

bool PlayGame::touch_pause(const Point &pt)
{
    (void)(pt);

    doPauseGame();
    return true;
}

bool PlayGame::touch_play(const Point &pt)
{
    _controlsPlay.touched(pt);    //needed to highlight a touched control

    if (_round.touch(pt))
        return true;

    if (_pause_rect.contains(pt))
    {
        doPauseGame();
    }
//    else if (_gd._gamemusic_icon.contains(pt) && Locator::audio().musicEnabled())
//    {
//        IAudio &a = Locator::audio();
//        a.musicMute(!a.musicEnabled());
//        _gd._gamemusic_icon.setFrame(a.musicEnabled()?0:1);    //first frame (on) or second frame (off)
//        a.pushPauseTrack();
//    }
	else
		if (!_doubleClick.done())
		    commandTryWord();       //same as click (?) button
		else
			_doubleClick.start(300);

    return true;
}
bool PlayGame::touch_end(const Point &pt)
{
    _controlsPlay.touched(pt);    //needed to highlight a touched control
	//test if word box selected (doubleclick to see word definition)
	const int yo = _yScratchBot + CURSORH + _boxOffsetY;	//start y offset
	//[3..], [4...], [5....] to [N.....] letter boxes
	int xx, yy;
	for (xx=_shortestWordLen; xx<=_longestWordLen; ++xx)	//accross the screen
	{
		//check if click even in this column (speed up)
		if (pt.x < _boxOffset[xx] || pt.x > _boxOffset[xx]+_boxLength[xx]) continue;

		int nWords = _gd._words.wordsOfLength(xx);
		if (nWords)	//column has words
		{
			for (yy=0; yy<nWords; ++yy)	//down the screen
			{
				Point pBox(_boxOffset[xx], yo+(yy*(BOXH + BOXHGAP)));     //same as in render_play()
				Rect r(pBox, pBox.add(Point(_boxLength[xx], BOXH)));
				if (r.contains(pt))
				{
					if (!_doubleClick.done())
						doDictionary();		//toggle dictionary mode
					else
					{
						_doubleClick.start(300);
						_xxWordHi = xx;
						_yyWordHi = yy;
					}

                    state_end_setup_scrollers();
					return true;
				}
			}
		}
	}

//	//uncomment to allow click somewhere else on screen to continue
//	if (!_doubleClick.done())
//		doMoveOn();
//	else
//		_doubleClick.start(300);

    return true;
}

//releasing 'touch' press
bool PlayGame::tap(const Point &pt)
{
    if (_play != nullptr)
    {
        return _play->tap(pt);
    }

    const int ctrl_id = _controlsPlay.tapped(pt);

    _round.tap(pt);

    //NOTE: tests must be done in this order for when popup is running
    //////////////////////////////////////////////////////////////////

    //must check buttons available during popup, otherwise they animate but
    //but don't update their state when pressed.
    if (ctrl_id == CTRLID_MENU)
    {
        //need to push a key command instaed of starting as it needs an Input ptr
        int key = Locator::input().un_translate(ppkey::SELECT);
        ppg::pushSDL_EventKey(key);
        return true;
    }
	else if (ctrl_id == CTRLID_MUSIC)
	{
	    Locator::audio().toggleMusic();
        return true;
	}
	else if (ctrl_id == CTRLID_SFX)
	{
	    Locator::audio().toggleSfx();
        return true;
	}

    //if popup running, handle those (after menu & sound buttons above)
	if (_pPopup)
	{
		_pPopup->tap(pt);
		handlePopup();
		return true;
	}

    //handle remaining buttons when not running popup
    if (ctrl_id == CTRLID_NEXT || ctrl_id == CTRLID_EXIT)
    {
        if (PG_END == _state)
        {
            doMoveOn();
            return true;
        }
    }
	else if (ctrl_id == CTRLID_LAST)
	{
		commandWordToLast();
	}
	else if (ctrl_id == CTRLID_TOTOP)
	{
		commandClearAllToTop();
	}
	else if (ctrl_id == CTRLID_SHUFFLE)
	{
		commandJumbleWord();
	}
	else if (ctrl_id == CTRLID_TRYWORD)
	{
		commandTryWord();
	}
	else if (ctrl_id == CTRLID_SCROLL_UP)
	{
	    button_end_up();
        return true;
	}
	else if (ctrl_id == CTRLID_SCROLL_DOWN)
	{
	    button_end_down();
        return true;
	}

    return false;
}


//command issued by player - to place last word in scratch panel
void PlayGame::commandWordToLast()
{
	//start pulse anim and launch command
	_round.setWordToLast();
}
//command issued by player - to place all chars in the top row
void PlayGame::commandClearAllToTop()
{
	_round.clearAllToTop();
}
//command issued by player - to jumble all remaining letters in the top row
void PlayGame::commandJumbleWord()
{
	if (_round.jumbleWord())
        Locator::audio().playSfx(AUDIO_SFX_JUMBLE);
}
//command issued by player - to check word selected against dictionary
void PlayGame::commandTryWord()
{
	tryWord();
}

//start the animation to slide the four round action buttons,
//from off screen, to their correct place beside the letters.
void PlayGame::slideRoundButtonsIn()
{
    if (_pPopup)
        return;    //not if popup visible (say before letters finish rolling in)

    //these four controls should always be created and available
    const int ms = 450;
    _controlsPlay.getControlSprite(CTRLID_SHUFFLE)->startMoveTo(_posRButtonLeft, _posRButtonTop, ms);
    _controlsPlay.getControlSprite(CTRLID_TRYWORD)->startMoveTo(_posRButtonLeft, _posRButtonBot, ms);
    _controlsPlay.getControlSprite(CTRLID_TOTOP)->startMoveTo(_posRButtonRight, _posRButtonTop, ms);
    _controlsPlay.getControlSprite(CTRLID_LAST)->startMoveTo(_posRButtonRight, _posRButtonBot, ms);

    //enable menu and sound buttons
//    _controlsPlay.enableControl(true, CTRLID_MENU);
//    _controlsPlay.enableControl(true, CTRLID_SFX);
//    _controlsPlay.enableControl(true, CTRLID_MUSIC);
}

//start the animation to slide the four round action buttons,
//beside the letters, to off screen
void PlayGame::slideRoundButtonsOut()
{
    //these four controls should always be created and available
    const int ms = 450;
	const int btnWidth = _controlsPlay.getControlSprite(CTRLID_SHUFFLE)->tileW();
    _controlsPlay.getControlSprite(CTRLID_SHUFFLE)->startMoveTo(-btnWidth, _posRButtonTop, ms);
    _controlsPlay.getControlSprite(CTRLID_TRYWORD)->startMoveTo(-btnWidth, _posRButtonBot, ms);
    _controlsPlay.getControlSprite(CTRLID_TOTOP)->startMoveTo(SCREEN_WIDTH, _posRButtonTop, ms);
    _controlsPlay.getControlSprite(CTRLID_LAST)->startMoveTo(SCREEN_WIDTH, _posRButtonBot, ms);

    //disable menu and sound buttons
//    _controlsPlay.enableControl(false, CTRLID_MENU);
//    _controlsPlay.enableControl(false, CTRLID_SFX);
//    _controlsPlay.enableControl(false, CTRLID_MUSIC);
}

//process any events not handled by the main input function
void PlayGame::handleEvent(SDL_Event &sdlevent)
{
	if (sdlevent.type == SDL_USEREVENT)
	{
		if (USER_EV_PING_COUNTDOWN == sdlevent.user.code)
		{
			if ( --_countdown < 11 || _pPopup)	//added pinger when in popup to warn user that timer still going...
			{
                Locator::audio().playSfx(AUDIO_SFX_PING);
			}
		}
		else
		if (USER_EV_END_MOVEMENT_ROUNDEL == sdlevent.user.code)
		{
		    //Event uses data1 as a simple int (as using a ptr to
            //sprite internal data may not exist after end of movement),
            //data2 is the roundel class id so we know which one if more than one animanting
            if (reinterpret_cast<int>(sdlevent.user.data2) == _round.getRoundelsId())
            {
                //slide round buttons onto screen. (slide off again for popup or end game etc)
                slideRoundButtonsIn();
            }
		}
		else
		if (USER_EV_END_COUNTDOWN == sdlevent.user.code)
		{
			if (!_bAbort && foundEnoughWords() 					//must be at least one 6 letter word found to continue
				&& !(_countdown==0 && GM_TIMETRIAL==_gd._mode))	//and not timetrial that has reached 0 (timetrial still going is ok though)
			{
				//if fastest show a flash of the bonus, maybe below the wordlist
				//just to tell user they got the fastest time... and a bonus...?
				_fastest = _fastestCountStart - _countdown;
				if (!_gd._score.setCurrFastest(_fastest)) _fastest = 0;	//reset to 0 if not the fastest (so we can display)
				if (_gd._score.currWords()==0) _fastest = 0;	//never bonus if first word, but has recorded it as fastest for scoreboard

				if (_gd._mode <= GM_REWORD && foundAllWords())
				{
					//wow! all words got so bonus and go to next level
                    Locator::audio().playSfx(AUDIO_SFX_ALLFOUND);
					const int bonus = SCORE_BONUS + (_countdown*SCORE_SECONDS);	//say: 100 + remaining seconds * 10
					_gd._score.addCurrScore(bonus);
					showSuccess(SU_BONUS, bonus);
				}
				else
				{
					//great! at least one 6 letter found so go to next level
					//If in SPEEDER or TIMETRIAL mode, only one 6 letter word needed to move on...

					//fill rest of found words list with words to be found and
					//display them in diff colour to show player what they missed
					fillRemainingWords();

                    Locator::audio().playSfx(AUDIO_SFX_FOUND6);

					if (_gd._mode <= GM_REWORD)
					{
					    //if arcade, set arcade bonus unless got a all-letter word, give reword bonus
					    const int bonus = (_maxwordlen==_longestWordLen)?SCORE_BONUS:SCORE_ARCADE;
						_gd._score.addCurrScore(bonus);
						showSuccess((_maxwordlen==_longestWordLen)?SU_GOT6:SU_ARCADE, bonus);
					}
					else
					{
						//it's GM_SPEEDER or GM_TIMETRIAL so add score and any fastest bonus,
						//then move to next word
						_gd._score.addCurrScore(SCORE_WORD6);
						if (_fastest) _gd._score.addCurrScore((maxCountdown()-_fastest)*SCORE_FASTEST);
						if (GM_SPEEDER == _gd._mode)
						{
							showSuccess(SU_SPEEDER);
						}
						else //GM_TIMETRIAL
						{
							showSuccess(SU_TIMETRIAL, _fastest);
						}
					}
				}
                _controlsPlay.showControl(true, CTRLID_MENU);
                _controlsPlay.showControl(false, CTRLID_EXIT);

                _controlsPlay.showControl(true, CTRLID_NEXT);
			}
			else
			{
				//oh dear, ran out of time (or player forced exit, or save&exit)

				//fill rest of found words with words to be found and
				//display them in diff colour to show player what they missed
				fillRemainingWords();

				if (_bAbort || _bSaveExit)
				{
				    if (_bSaveExit)
				    {
                        Locator::audio().playSfx(AUDIO_SFX_FOUND6);
                        showSuccess(SU_SAVEEXIT);
				    }
                    else
                    {
                        Locator::audio().playSfx(AUDIO_SFX_NOT6,2);	//play 3 times if no hiscore
                        showSuccess(SU_GAMEOVER);
                    }
				}
				else if (_gd._score.isHiScore(_gd._mode, _gd._diffLevel) != -1)
				{
					//excellent! player got on scoreboard
                    Locator::audio().playSfx(AUDIO_SFX_NOT6,1);	//play twice if got on hiscore
					showSuccess(SU_GAMEOVER);
				}
				else
				{
					//oh dear, ran out of time without a high enough score
                    Locator::audio().playSfx(AUDIO_SFX_NOT6,2);	//play 3 times if no hiscore
					showSuccess(SU_BADLUCK);
				}

                _controlsPlay.showControl(false, CTRLID_MENU);
                _controlsPlay.showControl(true, CTRLID_EXIT);

                _controlsPlay.showControl(false, CTRLID_NEXT);    //leave 0 countdown showing
			}

            //none of the round letter action buttons should now be visible
            _controlsPlay.showGroup(false, CTRLGRP_LETTERS);
		}
		else
		if (USER_EV_EXIT_SUB_SCREEN == sdlevent.user.code)
		{
		    //pop latest game state change (e.g PlayGameDict or PlayGamePause)
            statePop();
		}
	}
}

void PlayGame::doMoveOn()
{
    _controlsPlay.showControl(false, CTRLID_NEXT);
    _controlsPlay.showControl(false, CTRLID_EXIT);

	//play over so do we go to next level or back to menu...?
	switch (_success)
	{
    case SU_ARCADE:                 //Well done, you got enough words to continue
	case SU_GOT6:					//WELL DONE! - You got the 6
	case SU_SPEEDER:				//or if in Speeder mode, a 6 entered so move onto next level
	case SU_BONUS:					//BONUS!! - You got all words
		statePop(); 				//out of PG_END state
		newLevel();
		break;
	case SU_TIMETRIAL:				//in time trial mode quickly move on to next level immediately
		if (_countdown==0)
		{
			_bAbort = true;
			ppg::pushSDL_Event(USER_EV_END_COUNTDOWN); //try to exit more gracefully, pushes end of level
		}
		else
		{
			statePop(); 			//out of PG_END state
			newLevel();
		}
		break;
	case SU_BADLUCK:				//Bad Luck! - Out of time
		exit(ST_MENU);
		break;
	case SU_SAVEEXIT:
        _gd.saveQuickState();       //save before back to menu
        exit(ST_MENU);
        break;

	default:
	case SU_GAMEOVER:				//Game Over - You're a high scorer!
        if (_gd._score.isHiScore(_gd._mode, _gd._diffLevel) != -1)
            exit(ST_HIGHEDIT);
        else
            exit(ST_MENU);
		break;
	}
}
//toggle dictionary mode at end of level to allow player
//to see the full definition of a word
void PlayGame::doDictionary()
{
	if (PG_DICT == _state)
	{
		//put state back to end of level state as that's the
		//only state we can get to dictionary mode from
		statePop(); //_state = PG_END;
	}
	else if (PG_END == _state) 	//fix, was doing dict in PAUSE
	{
		//get the word currently highlighted
        _dictWord.empty();
        const int yyOffset = _boxWordOffset[_xxWordHi];
		tWordsFoundList::const_iterator it;
		it = _wordsFound[_xxWordHi].begin();
		for (int yy=0; yy<(int)_wordsFound[_xxWordHi].size(); ++yy)	//find the matching word in the list
		{
			if (_yyWordHi + yyOffset == yy && it != _wordsFound[_xxWordHi].end())
			{
				_dictWord = (*it)._word;
				break;
			}
			++it;
		}

		//display current highlighted word on separate 'screen'
		statePush(PG_DICT); //_state = PG_DICT;
	}
}

void PlayGame::doPauseGame()
{
	//check if already paused
	if (PG_PAUSE == _state)
	{
		//is paused so unpause
		statePop();

		//penalty for pause, at least 10 seconds used
		if (_countdown > 10) _countdown-=10;

		//release the countdown timer
		startCountdown();
		return;
	}
	//else is not paused, so check if we can pause

	//if not in play mode dont need to pause the game
	if (PG_PLAY != _state) return;

	//only on easy/med mode, or at least one 6 letter word found on this level
	//or enough letters/words in arcade mode entered.
	if (_gd._diffLevel < DIF_HARD || foundEnoughWords())
	{
		//pause the countdown timer
		stopCountdown();

		statePush(PG_PAUSE);

		//prepare roundel class ready for a pause
		tSharedImage &letters = Resource::image("roundel_letters.png");
		_roundPaused = tAutoRoundels(new Roundels());
		const int y = (SCREEN_HEIGHT/2)-letters.get()->height();
		_roundPaused->setWordCenterHoriz(std::string("PAUSED"), letters, y, 4);
		_roundPaused->easeMoveFrom(Screen::width(), 0, 800, 40, Easing::EASE_OUTQUART);

		Mix_FadeOutChannel(-1, 1000);
	}
}

void PlayGame::newGame()
{
	//Y pos of scratch area, doesn't change during game so calc here first
	//   (X pos depends on number of letters in word so calculated in newLevel()
	//scratch letter area _yScratchTop (6 roundels on top)
	//scratch box area _yScratchBot (6 boxes below roundels)
	_yScratchTop = _scorebar.height() + GAME_GAP1;
	_yScratchBot = _yScratchTop + CURSORH + GAME_GAP1;

	_gd._score.resetCurr();
	if (_gd._state == ST_RESUME)
	{
		_gd.loadQuickState();
		_gd._state = ST_GAME;	//as if it was always...
	}
	_gd._unmatchedWords.clear();

	if (_gd._mode == GM_TIMETRIAL)
	{
		//set initial timetrial counter as each level does not reset the timer
		_countdown = maxCountdown();
	}

	Mix_FadeOutChannel(-1, 1000);	//fade out menu music & sound effects etc
////	Mix_FadeOutMusic(1000);	//and any music	##TODO##

}

//timer specific to the countdown timer
//no need for anything else in this game
void PlayGame::startCountdown()
{
	//(xthou / 10) * 10;  /* To round it down to the nearest 10 ms */

	Uint32 rate = 1000; //1 second
	stopCountdown();	//make sure its stopped first
	_countdownID = SDL_AddTimer(rate, countdown_callback, nullptr);
}

//stop the countdown timer redrawing the screen
void PlayGame::stopCountdown()
{
	if (_countdownID)
	{
		if (SDL_RemoveTimer(_countdownID)) _countdownID = 0;
		clearEventBuffer();

	  // Fade out all the audio channels in 1 second
//	  Mix_FadeOutChannel(-1, 1000);

		// and the music in 1 second
	  //Mix_FadeOutMusic(1000);	//dont have any right now

	  // wait one second for fade to finish
//	  SDL_Delay(1000);

	}
}

//clear any unused events from the event queue
//used to prevent timer going off after being closed etc
void PlayGame::clearEventBuffer()
{
	SDL_Event sdlevent;
	while (SDL_PollEvent(&sdlevent));	//empty event buffer
}

//define the number of seconds each difficulty level gets (in global.h)
int PlayGame::maxCountdown()
{
//#ifndef _DEBUG
	if (_gd._mode == GM_TIMETRIAL)
		return ((int)DIF_MAX - _gd._diffLevel)*COUNTDOWN_TIMETRIAL;
	if (_gd._mode == GM_SPEEDER)
		return ((int)DIF_MAX - _gd._diffLevel)*COUNTDOWN_SPEEDER;
	//else its REWORD
	return ((int)DIF_MAX - _gd._diffLevel)*COUNTDOWN_REWORD;
//#else
//	return 30;	//##DEBUG##
//#endif
}

bool PlayGame::newLevel()
{
	stopCountdown();

	int xx;
	for (xx=0; xx<=TARGET_MAX; ++xx)
	{
		_wordsFound[xx].clear();
		_boxWordOffset[xx]=0;
	}

	std::string newword;
	//nextWord() returns false if bad dictionary entry (XXXXXX corrupted or hacked) !
	if (!_gd._words.nextWord(newword, _gd._diffLevel, _gd._mode))	//return next word found.
	{
        std::cerr << "Cannot load new level word" << std::endl;
	    exit(ST_MENU);
	    return false;
	}
    _longestWordLen = newword.length();
	_shortestWordLen = _longestWordLen-(MAX_WORD_ROW-1); //say if longest is 6, then shortest is 3 (for 3, 4, 5, 6)

	//X pos of scratch area depends on length of word so calc here at each new level/word)
	_xScratch = (SCREEN_WIDTH - ((_longestWordLen * (CURSORW+2)) -2) ) /2;

	_boxOffsetY = 9;	//default y offset under scratch area - reset by speeder and timetrial to be slightly lower

	//simple algo for now just using box offset and len - until found boxes wrap around or whatever...
	int xLen = 0;	//length of all word boxes added together
	int n = 0;
	//only ever have to find 4 word lengths for any given target word length
	//i.e. For 6 letter target - find 3,4,5,6. For 7 letter target - find 4,5,6,7 etc
	for (n=_shortestWordLen; n<=_longestWordLen; ++n)	//4 word lengths inc target len
		xLen += (n*FOUND_WORD_CHR);	//eg xxx=40, xxxx=50, xxxxx=60, xxxxxx=70, etc

	//equal gaps = screen W - col width / num gaps + 1
	int xGap = (SCREEN_WIDTH - xLen) / (_longestWordLen-_shortestWordLen+2);
	int nextPos = xGap;
	for (n=_shortestWordLen; n<=_longestWordLen; ++n)
	{
		xLen = (n*FOUND_WORD_CHR);	//xxx=40, xxxx=50, xxxxx=60, xxxxxx=70, etc

		if (_gd._mode <= GM_REWORD)	//classic mode
		{
			_boxOffset[n] = nextPos;
			_boxLength[n] = xLen;
			nextPos = _boxOffset[n] + _boxLength[n] + xGap;
		}
		else	//GM_SPEEDER or GM_TIMETRIAL
		{
			//current_w is not supported in my gp2x SDL build, so saved from video setup
//			const SDL_VideoInfo *pVI = SDL_GetVideoInfo();	//for pVI->current_w
			_boxOffset[n] = _boxLength[n] = 0;
			if (n == _longestWordLen)
			{
				//middle of screen as only a single N letter word column
				_boxOffset[n] = (_gd._current_w / 2) - (_boxes.tileW() / 2);
				_boxLength[n] = xLen;

				_boxOffsetY = 30;	//slightly lower as it has only 1 box
			}
		}
	}

	_round.setWord(newword, Resource::image("roundel_letters.png"), _xScratch+2, _yScratchTop+2, 6, true);
	_round.setBottomPos(_xScratch+2, _yScratchBot+2);
	_round.jumbleWord(false);		//randomize the letters
	_round.easeMoveFrom(Screen::width(), 0, 800, 150, Easing::EASE_OUTQUART);//animate roundels into screen pos

	//paint this levels word boxes onto the background for quick blit display rather
	//than having to loop through and blit each box every frame as in prev versions
	prepareBackground();

	//if in REWORD or SPEEDER then reset the timer. TimeTrial keeps counting down...
	if (_gd._mode != GM_TIMETRIAL)
		_countdown = maxCountdown();	//60 seconds for hard, 120 for med, 180 for easy

	_fastestCountStart = _countdown;	//either continuing in TIMETRIAL, or else restarted
	_success = SU_NONE;
	_bonusScore = 0;
	_fastest = 0;
	_xxWordHi = _longestWordLen;	//highlight the target word column (for dict definition on end of level)
	_yyWordHi = 0;			//highlight first 6 letter word in column
	_maxwordlen = 0;		//always reset the fact that a 6 letter word not got yet

	//prepare random 'success' title here ready for end of level
	_randomTitle = g_randInt.random(4);		//0..3

	//onscreen touch command icons
	const int btnWidth = _controlsPlay.getControlSprite(CTRLID_SHUFFLE)->tileW();
   	_posRButtonLeft = 12;
	_posRButtonRight = SCREEN_WIDTH-btnWidth-12;	//all these touch incons same size
	_posRButtonTop = _yScratchTop+2+((LETTERH-btnWidth)/2);	//all same size
	_posRButtonBot = _yScratchBot+2+((LETTERH-btnWidth)/2);
	//but always start off screen so can be slid on (slideRoundButtonsIn() etc)
	_controlsPlay.getControlSprite(CTRLID_SHUFFLE)->setPos(-btnWidth, _posRButtonTop);
	_controlsPlay.getControlSprite(CTRLID_TRYWORD)->setPos(-btnWidth, _posRButtonBot);
	_controlsPlay.getControlSprite(CTRLID_TOTOP)->setPos(SCREEN_WIDTH, _posRButtonTop);
	_controlsPlay.getControlSprite(CTRLID_LAST)->setPos(SCREEN_WIDTH, _posRButtonBot);
    _controlsPlay.showGroup(true, CTRLGRP_LETTERS);

    calcArcadeNeededWords();    //arcade mode highlights

    state_end_setup_scrollers();    //removes any scroll buttons used in end game state

	startCountdown();
	clearEventBuffer();	//start fresh each level
	return true;
}

//Calc how many words in each column needed to continue (in arcade mode)
//Called after each word is found by player so new highlights can be drawn
void PlayGame::calcArcadeNeededWords()
{
    memset(_boxWordNeeded, 0, sizeof(_boxWordNeeded));

    if (GM_ARCADE == _gd._mode)
    {
        //depending on difficulty, need at least n words of each type to continue
        //unlike Classic (Original Reword) mode that needs only a single all-letter
        //word to continue.

        int xx(0), yy(0), nTotalLetters(0);
        //1. tot up the total number of letters in words available
        for (xx=_shortestWordLen; xx<=_longestWordLen; ++xx)
        {
            //only count nWords on screen, not overflow
            const int nWordsOnScreen = std::min(_gd._words.wordsOfLength(xx), MAX_WORD_COL);
            nTotalLetters += nWordsOnScreen * xx;     //words * numChars
        }
        _debugTotalLetters = nTotalLetters; //for debug display

        //2. calc number of letters needed to continue (% based on difficulty)
        //for easy, need to find 25% of all words, or an all-letter word
        //for med, need to find 35% of all words, or an all-letter word
        //for hard, need to find 45% of all words, or an all-letter word
        const int pc = (DIF_EASY == _gd._diffLevel)?25:(DIF_MED == _gd._diffLevel)?35:45;
        int needed = (nTotalLetters * ((float)pc/100));
        _debugNeededAll = needed; //for debug display

        for (xx=_longestWordLen; xx>=_shortestWordLen; --xx)
        {
            //remove letters (in words) already found
            needed -= (int)_wordsFound[xx].size() * xx;
        }
        _debugNeededNow = needed;

        bool bAtLeaseOne = false;
        for (xx=_shortestWordLen; xx<=_longestWordLen; ++xx)
        {
            //only count nWords on screen, not overflow
            const int nWords = std::min(_gd._words.wordsOfLength(xx), MAX_WORD_COL);
            for (yy = 0; yy < nWords; ++yy)
            {
                if (needed >= xx)
                {
                    _boxWordNeeded[xx]++;
                    needed -= xx;
                    bAtLeaseOne = true;
                    if (xx == _longestWordLen) break; //only need to show one longest word
                }
            }
        }
        //now try and ensure we never get a situation where % so low no boxes are needed and
        //whole box grid turns green before user plays any words! Also make sure we don't re-yellow
        //the longest word.
        if (!bAtLeaseOne)
        {
            //find first valid column and one to get
            for (xx=_shortestWordLen; xx<=_longestWordLen; ++xx)
            {
                const int nWords = std::min(_gd._words.wordsOfLength(xx), MAX_WORD_COL);
                if (!nWords) continue; //no words in this column so ignore

                if (_wordsFound[xx].size() == 0) //none found for this col yet
                {
                    if (xx != _longestWordLen)  //and not the re-word col
                        _boxWordNeeded[xx]++;   //so force at least one word
                }
                break;
            }
        }
    }
}

//show a SUCCESS! screen over play area before continuing with next level
//passes in one of the eSuccess types: SU_GOT6, SU_BONUS, SU_BADLUCK, SU_GAMEOVER
void PlayGame::showSuccess(eSuccess newSuccess, int newBonus)
{
	if (SU_TIMETRIAL != newSuccess)
		stopCountdown();

	statePush(PG_WAIT); //_state = PG_WAIT;
	_success = newSuccess;
	_bonusScore = newBonus;

	//make sure player doesn't clear the screen before its seen
	clearEventBuffer();
	_waiting.start(500);	//half second before user can continue after level done
}

// Submit the current word
void PlayGame::tryWord()
{
	int wordlen = tryWordAgainstDict();	    //0=already found, -1=not a 6 or in sub word list

	 //speeder and timetrial only need a 6 to continue
	if (GM_TIMETRIAL == _gd._mode || GM_SPEEDER == _gd._mode)
	{
		if (_longestWordLen == wordlen)
		{
			_gd._score.addCurrWords(1);
			ppg::pushSDL_Event(USER_EV_END_COUNTDOWN);	//pushes end of level
			return;
		}
		else wordlen = -1;	//speeder or timetrial badWord sound as < 6 letters
	}

 	//must be GM_ARCADE or GM_REWORD or bad word
	if (wordlen >= _shortestWordLen && wordlen <= _longestWordLen)	//is 3,4,5, or 6 (or whatever)
	{
	    if ( _gd._words.wordsOfLength(_longestWordLen) == 1)
	    {
	        //found first reword for this level so add 1 to word count
			_gd._score.addCurrWords(1);
	    }

		_gd._score.addCurrScore( (_longestWordLen == wordlen)?SCORE_WORD6:SCORE_WORD );	//more for a 6 letter word
		_round.clearAllToTop(true);	//remove hit word

		if (foundAllWords())
		{
			ppg::pushSDL_Event(USER_EV_END_COUNTDOWN); //pushes end of level
			return;	//exit before sound, as success() plays fanfare sound
		}
		//it's a simple word find
        Locator::audio().playSfx((_longestWordLen == wordlen)?AUDIO_SFX_FOUND6:AUDIO_SFX_FOUNDNON6);

		calcArcadeNeededWords();
	}
	else
	{
        Locator::audio().playSfx((0 == wordlen)?AUDIO_SFX_ALREADYDONE:AUDIO_SFX_NOTINDICT);
		_round.clearAllToTop(false);	//remove bad word - dont move cursor
	}
}

//ok, match whatever's in the bottom row with our list of words
//that are made up from the top row letters/word
//Returns the number of letters matched in a word (ie 3,4,5 or 6 letter word)
//Return 0 if a word already used, or -1 if its not in the list at all
//Also saves curr bottom word in "last" array so we can repopulate it if player asks
int PlayGame::tryWordAgainstDict()
{
	std::string newword = _round.getBottomWord();

	int ret = _gd._words.checkWordsInTarget(newword);
	if (ret == -1 && newword.length() > 2) _gd._unmatchedWords.insert(newword);	//save for add to personal dict
	if (ret < 1) return ret; //0=already found, -1=not found
	//else found in target and is set to 'found' in list

	const int w = newword.length();

	//add found word to (front of) list of words of that length already found
	//(and so be displayed in reverse found order in the boxes under the main letters)
	DictWord dictWord;
	dictWord._word = newword;
	dictWord._found = true;
	_wordsFound[w].push_front(dictWord);

	std::sort(_wordsFound[w].begin(), _wordsFound[w].end(), DictWord());

	//save max len word so far
	if ((int)newword.length() > _maxwordlen) _maxwordlen = newword.length();

	return newword.length();
}

//return true if a longest-word found or, in arcade mode, if words needed list is empty
bool PlayGame::foundEnoughWords()
{
    if (_maxwordlen==_longestWordLen)
        return true;

    if (_gd._mode == GM_ARCADE)
    {
        int iCount(0);
        for (int xx=_shortestWordLen; xx<=_longestWordLen; iCount += _boxWordNeeded[xx++]);
        if (iCount == 0)
            return true;
    }

    if (foundAllWords())
        return true;

    return false;
}

//return true if all words available for the 6 letter word have been found
bool PlayGame::foundAllWords()
{
	bool bAllWords = true;
	int xx;
	for (xx=_shortestWordLen; xx<=_longestWordLen; ++xx)
		//if max words found, or all words on screen (if some are off screen) found
		if ((_gd._words.wordsOfLength(xx) <= MAX_WORD_COL && _gd._words.wordsOfLength(xx)!=(int)_wordsFound[xx].size())
			|| (_gd._words.wordsOfLength(xx) > MAX_WORD_COL && _wordsFound[xx].size() < MAX_WORD_COL))
			bAllWords = false;
	return bAllWords;
}

//now at end of level so fill the remaining entries with the words not yet found
void PlayGame::fillRemainingWords()
{
	const tWordsInTarget wit = _gd._words.getWordsInTarget();
	tWordsInTarget::const_iterator it;
	tWordsFoundList::iterator foundit;
	int w;
	std::string wrd;
	bool bFound;
	DictWord dictWord;

	for (it=wit.begin(); it != wit.end(); ++it)
	{
		wrd = (*it).first.c_str();
		w = wrd.length();
		bFound = false;
		for (foundit = _wordsFound[w].begin(); foundit != _wordsFound[w].end(); ++foundit)
		{
			if ((*foundit)._word == wrd)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			//we only need the word and "not found"
			dictWord._word = wrd;
			dictWord._found = false;
			_wordsFound[w].push_front(dictWord);
		}
	}

	//do each 3, 4, 5 & 6 word group and sort column of words into alpha order
	for (w=_shortestWordLen; w<=_longestWordLen; ++w)
		std::sort(_wordsFound[w].begin(), _wordsFound[w].end(), DictWord());
}

void PlayGame::prepareBackground()
{
	//create the background to be used for this level,
	//pre drawing so we dont need to do it each frame.
	//...

    Surface tmpSurface; //to build background before convert to texture (at end)
	tmpSurface.create(Screen::width(), Screen::height());
    ppg::drawSolidRect (&tmpSurface, 0, 0, Screen::width(), Screen::height(), GAMEBG_COLOUR);

	//place score bar centered - in case screen bigger than graphic
	int sb_x = (SCREEN_WIDTH - _scorebar.width())/2;  //in case sb w < screen w
	int sb_w = _scorebar.width();
	int sb_h = _scorebar.height();
	ppg::blit_surface(_scorebar.surface(), nullptr, tmpSurface.surface(), sb_x, 0);

	//prerender the score and words titles
	//find out sizes and calc reasonable positions
	FontTTF &fontScore = _gd._fntClean;
	FontTTF &fontCounter = _gd._fntBig;
	Rect r(0, 0, 0, 0);
	int score_len(0), score0_len(0), words_len(0), words0_len(0);
	r = fontScore.calc_text_metrics("SCORE: ");		//note gap to look better
	score_len = r._max.x;
	r = fontScore.calc_text_metrics("00000000");	//8 0's not drawn here
	score0_len = r._max.x;
	r = fontScore.calc_text_metrics("WORDS: ");		//note gap to look better
	words_len = r._max.x;
	r = fontScore.calc_text_metrics("0000");		//4 0's not drawn here
	words0_len = r._max.x;

	//now calc where we can place these and the gap between each
    //
    // |-------------------------------------------------------------------------------------|
    // |   /--------\                                                          /-----------\ |
    // |   |  MENU  |  () ()       SCORE:00000000         WORDS:0000           |    999    | |
    // |   \--------/                                                          \-----------/ |
    // |-------------------------------------------------------------------------------------|
    //  <  edge_pad >< equal_gap >             <equal_gap>          <equal_gap><  edge_pad  >
    //                             ^score_x               ^words_x
    //        ^_menu_icon_x             ^_score0_x              ^_words0_x           ^_countdown0_x
    //

    int middle_len = score_len+score0_len + words_len+words0_len;
    int edge_pad = sb_x + _controlsPlay.getControlSprite(CTRLID_NEXT)->tileW()+3; //space taken up by [ MENU ] or [ 999 ]

    int sound_gap(0);   //space between (mus) (fx)
    int sound_w(0), gamemusic_icon_x(0), gamefx_icon_x(0);
    Control *p = _controlsPlay.getControl(CTRLID_MUSIC);
    if (p)
    {
        sound_w = p->getSprite()->tileW();
        sound_gap = sound_w / 2.5;

        int soundIcon_y = (sb_h - p->getSprite()->tileH()) / 2;
        gamemusic_icon_x = edge_pad + sound_gap;
        p->getSprite()->setPos(gamemusic_icon_x, soundIcon_y);

        //must have done music to do fx
        gamefx_icon_x = gamemusic_icon_x + sound_w + sound_gap;
        p = _controlsPlay.getControl(CTRLID_SFX);
        if (p)
            p->getSprite()->setPos(gamefx_icon_x, soundIcon_y);

        sound_w = (gamefx_icon_x + sound_w) - gamemusic_icon_x;
    }
    int sound_right = sb_x + edge_pad + sound_w;    //right edge of sound icons () ()

	int equal_gap = (sb_w - middle_len - (edge_pad*2) - sound_w) / 3;    //space on bar len after edges removed, /3 for equal dist

	int titles_y = ((sb_h - fontText.height()) / 2) + 2;	//+2 magic number - too high otherwise...?
	int numbers_y = ((sb_h - fontNumbers.height()) / 2) + 2;	//+2 magic number - too high otherwise...?

	int score_x = sound_right + equal_gap;          //x pos for "SCORE:"
	_score0_x = score_x+score_len;                  //x pos for score "00000000"
	_score0_y = numbers_y;

	int words_x = _score0_x+score0_len+equal_gap;   //x pos for "WORDS:"
	_words0_x = words_x+words_len;                  //x pos for words "0000"
	_words0_y = numbers_y;

	_countdown0_x = (sb_x + sb_w) - (edge_pad / 2);
	_countdown0_y = (sb_h - fontCounter.height()) / 2;

	fontText.put_text(&tmpSurface, score_x, titles_y, "SCORE:", WHITE_COLOUR);
	fontText.put_text(&tmpSurface, words_x, titles_y, "WORDS:", WHITE_COLOUR);

	//draw mode and difficulty in bot right corner
	//(before/under word boxes so if 8 6-letter words, it doesnt cover boxes up)
	std::string strMode;
	Surface modeSurface;
	switch (_gd._mode)
	{
	case GM_ARCADE:		modeSurface.load("game_arcade.png");	_strMode = "ARCADE (" + _gd._diffName + ")";    break;
	case GM_REWORD:		modeSurface.load("game_reword.png");	_strMode = "REWORD (" + _gd._diffName + ")";    break;
	case GM_SPEEDER:	modeSurface.load("game_speeder.png");   _strMode = "SPEEDWORD (" + _gd._diffName + ")"; break;
	case GM_TIMETRIAL:	modeSurface.load("game_timetrial.png"); _strMode = "TIMETRIAL (" + _gd._diffName + ")"; break;
	default:break;
	}
	assert(modeSurface.surface());
	modeSurface.setAlphaTransparency(100);
	int mode_x = SCREEN_WIDTH - modeSurface.width() + 25; 	//slightly off screen
	int mode_y = SCREEN_HEIGHT - modeSurface.height() + 25;	//ditto
	ppg::blit_surface(modeSurface.surface(), nullptr, tmpSurface.surface(), mode_x, mode_y);

	//draw difficulty level in bot right corner
	std::string strDiff = strMode + " (" + _gd._diffName + ")";
	_gd._fntTiny.put_text_right(&tmpSurface, 4, SCREEN_HEIGHT - _gd._fntTiny.height() - 4, strDiff.c_str(), _gd._diffColour);

    //now make texture to be rendered as background during actual play/update
	_gamebg = tSharedImage(new Image(tmpSurface));
}
