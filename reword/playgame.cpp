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
										seperate functions for each render/work state.
									Added pinger when in popup menu, to warn user
				0.5		28.05.08	Added touchscreen support
				0.5.1	02.10.08	Add Pandora and other device screen layout/sizes


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

#include "playgamedict.h"

#include <cassert>
#include <algorithm>

//#include <SDL_gfxPrimitives.h>

static	int _countdown;				//seconds remaining

//function called when timer reaches interval set
Uint32 countdown_callback(Uint32 interval, void *param)
{
	if (_countdown <= 0)
	{
		pp_g::pushSDL_Event(USER_EV_END_COUNTDOWN); //pushEndOfLevel();
		return 0;
	}

	pp_g::pushSDL_Event(USER_EV_PING_COUNTDOWN);

	return(interval);
}

PlayGame::PlayGame(GameData& gd) : _gd(gd), _pPopup(NULL), _play(NULL)
{
#ifdef _DEBUG
	_dbg_display = false;
#endif
	_inputL  = _inputR = false;	//keys pressed + stick click (GP2X) to exit game
	_running = false;	//not init yet
	_init = false;		//ditto
	_bAbort = false;	//until L+R+CLICK which exits game to hiscore if needed or to menu
	_tmpDefMore = false;

	statePush(PG_PLAY);		//also sets default to PG_PLAY
}

PlayGame::~PlayGame()
{
    stopCountdown();
    delete _pPopup;
    delete _play;
}

void PlayGame::init(Input *input)
{
	//fade out any menu music (but only if no game music still playing)
	//Game music handled seperately froim in-game music (mp3 dir etc ?)
    if (Locator::GetAudio().isPlayingMusic()==false)
		Mix_FadeOutMusic(3000);

	//once the class is initialised, init and running are set true
	newGame();	//reset scores etc
	newLevel();	//preparebackground, get next word etc

    _nWordBoxHighlightOffset = (_gd._boxes.tileCount()/4); //4 blocks of word boxes
    _nWordBoxEmptyOffset = ((_gd._boxes.tileCount()/4)*2); //4 blocks.. 3rd block
    _nWordBoxNeededOffset = ((_gd._boxes.tileCount()/4)*3); //4 blocks.. 4th block

	//set the repeat of the keys required
	input->setRepeat(pp_i::UP, 250, 250);		//button, rate, delay
	input->setRepeat(pp_i::DOWN, 250, 250);
	input->setRepeat(pp_i::LEFT, 250, 250);
	input->setRepeat(pp_i::RIGHT, 250, 250);

    //[MENU] shows in top left (unless [EXIT] shown)
    boost::shared_ptr<Sprite> pMenu(new Sprite(RES_BASE + "images/touch_menu.png", 255, 4));
    pMenu->setPos(3, 0);
    pMenu->setTileSize(106, 52, Image::TILE_VERT);
    pMenu->setFrameLast();  //unselected
    pMenu->setVisible(true);  //menu option always visible during game (unless EXIT to be shown)
    Control cMenu(pMenu, CTRLID_MENU);
    _controlsPlay.add(cMenu);

    //[EXIT] goes in same place as [MENU] when game over
    boost::shared_ptr<Sprite> pExit(new Sprite(RES_BASE + "images/touch_exit.png", 255, 4));
    pExit->setPos(3, 0);
    pExit->setTileSize(106, 52, Image::TILE_VERT);
    pExit->setFrameLast();  //unselected
    pExit->setVisible(false);  //not available until countdown finished
    Control cExit(pExit, CTRLID_EXIT);
    _controlsPlay.add(cExit);

    //[NEXT] goes over the countdown position
    boost::shared_ptr<Sprite> pNext(new Sprite(RES_BASE + "images/touch_next.png", 255, 4));
    pNext->setPos(Screen::width() - pNext->width() - 3, 0);
    pNext->setTileSize(106, 52, Image::TILE_VERT);
    pNext->setFrameLast();  //unselected
    pNext->setVisible(false);  //not available until countdown finished
    Control cNext(pNext, CTRLID_NEXT);
    _controlsPlay.add(cNext);

    //bounding rect of [NEXT} button placed over countdown timer
    //doesn't move so can be cached here
    _pause_rect = pNext->bounds();

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
	_running = false;
}

//do not call directly. Use statePush() & statePop()
void PlayGame::stateFn(eState state)
{
    if (_play)
    {
        delete _play;
        _play = NULL;
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
					break;

    case PG_DICT:   {
                    _play = new PlayGameDict(_gd, _dictWord);
                    Input &i = static_cast<Input&>(Locator::GetInput());
                    _play->init(&i);
                    }
                    break;

	case PG_PAUSE:	pRenderFn = &PlayGame::render_pause;
					pWorkFn = &PlayGame::work_pause;
					pButtonFn = &PlayGame::button_pause;
					pTouchFn = &PlayGame::touch_default;
					break;
//	case PG_PAUSE:	_play = new PlayGamePause(_gd, _dictWord);
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
        _play->render(s);
        return;
    }

	//depending on state, call the function pointer of the
	//correctly mapped render function
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
	_gamebg->blitTo(s);

 	//draw scores and coloured seconds countdown
	_gd._fntSmall.put_number(s, _score0_x, _score0_y, _gd._score.currScore(), "%08d", BLACK_COLOUR);	//SCORE:
	_gd._fntSmall.put_number(s, _words0_x, _words0_y, _gd._score.currWords(), "%04d", BLACK_COLOUR);	//WORDS:
	//(>10) normal countdown in "plenty of time" colour
	//(<=10) countdown in "oh crap" colour (red to denote time running out)
	//		A warning "ping" is also sounded in the countdown callback fn each second...


	if (PG_PLAY == _state)
	{
        //center the countdown text in the scorebar countdown [area]
        _gd._fntBig.put_number_mid(s, _countdown0_y, _countdown0_x, //s->surface()->w - 54,
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
			_gd._scratch.blitTo(s, _xScratch+(xx*(CURSORW+2)), _yScratchBot, nTile);
		}

		//draw touch controls
		_gd._word_last_pulse.draw(s);
		_gd._word_totop_pulse.draw(s);
		_gd._word_shuffle_pulse.draw(s);
		_gd._word_try_pulse.draw(s);
        _gd._gamemusic_icon.draw(s);

		//draw game letters
		_round.draw(s);

		_gd._cursor.blitTo(s, _xScratch+(_round.currentX()*(CURSORW+2)),
						(_round.cursorIsTop()?_yScratchTop:_yScratchBot), (int)_gd._diffLevel-1);
	}

	//draw word boxes 1 length at a time downwards (easier)
	const int yo = _yScratchBot + CURSORH + _boxOffsetY;	//start y offset
	tWordsFoundList::const_iterator it;

	//[3..], [4...], [5....] and [6.....] letter boxes
	for (xx=_shortestWordLen; xx<=_longestWordLen; ++xx)	//accross the screen
	{
		it = _wordsFound[xx].begin();
		const int nWords = _gd._words.wordsOfLength(xx);
		const bool bHighlightWholeColumn = (PG_PLAY == _state && _round.getBottomWordLength() == xx);

		if (nWords)	//column has words
		{
			for (yy=0; yy<nWords; ++yy)	//down the screen
			{
			    const int boxOffsetY = yo+(yy*(BOXH + BOXHGAP));  //same as in touch_end()

				//As the old limit of 7 on screen word boxes has now been extended to 8, we can't use
				//this fuction (as boxes reach to the very bottom of the screen). If it ever goes back
				//to 7 and includes more words than can fit in a 7 word column, we might use this again.
				if (yy >= MAX_WORD_COL)
				{
					//? there are more words than can fit on screen (7 down)
					//  so show remaining count which turns green when all words are found
					//NOTE: this is currently not used as the dictionary file only has words
					//		with up to seven 3, 4, 5 and 6 letter words that make up those available
					//		Just use a dictionary with > 7 words in any 3, 4, 5 or 6 letter words
					//		for this functionality to work
//#ifdef _DEBUG
					if (PG_PLAY == _state)
						_gd._fntClean.put_number(s, _boxOffset[xx]+6, boxOffsetY, nWords-_wordsFound[xx].size(), "%d",
							(nWords==(int)_wordsFound[xx].size())?GREEN_COLOUR:BLUE_COLOUR);

					//render the "more" arrow under column
//					_gd._moreWords
//#endif
					break;	//out of this n letters loop
				}

				//draw the box for the word to be displayed in - using a highlighted box if
				//not in play state and this is the curr word the selection is on.
//				_gd._boxes.blitTo( s, _boxOffset[xx], boxOffsetY,					//tile 0=3, 1=4, 2=5, 3=6 etc. letter words
//					(
//                        ((PG_PLAY != _state && xx == _xxWordHi && yy == _yyWordHi) || bHighlightWholeColumn) ?
//                            xx+_nWordBoxHighlightOffset :  //xx+(count/4) for n box blocks in boxes.png
//                            (PG_PLAY == _state && yy < _boxWordNeeded[xx]) ?
//                                xx+_nWordBoxNeededOffset :
//                                xx
//                    )-3 );  //-3 to reset the xx back to 0 as the boxes.png starts at 3 letter tile
//
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
                    _gd._boxes.blitTo( s, _boxOffset[xx], boxOffsetY,					//tile 0=3, 1=4, 2=5, 3=6 etc. letter words
                        (
                            ((PG_PLAY != _state && xx == _xxWordHi && yy == _yyWordHi) || bHighlightWholeColumn) ?
                                xx+_nWordBoxHighlightOffset :  //xx+(count/4) for n box blocks in boxes.png
                                (PG_PLAY == _state && _maxwordlen!=_longestWordLen && yy < _boxWordNeeded[xx]) ?
                                    xx+_nWordBoxNeededOffset :      //yellow
                                    xx //+_nWordBoxHighlightOffset     //green
                        )-3 );  //-3 to reset the xx back to 0 as the boxes.png starts at 3 letter tile

					//Display the word in red (not found) or blue (found)
					//Only found words populate the container during play, so red only drawn at end of level
					_gd._fntClean.put_text(s, _boxOffset[xx]+BOXTEXTOFFSETX, boxOffsetY+BOXTEXTOFFSETY,
							(*it)._word.c_str(), ((*it)._found)?BLUE_COLOUR:RED_COLOUR);

					++it;   //next found word
				}
				else
                {
                    //no word for this box so just empty (but possibly highlight for Arcade 'to be found' box)
                    //or show in red if end of level (not in-play)
                    _gd._boxes.blitTo( s, _boxOffset[xx], boxOffsetY,					//tile 0=3, 1=4, 2=5, 3=6 etc. letter words
                        (
                            ((PG_PLAY != _state && xx == _xxWordHi && yy == _yyWordHi) || bHighlightWholeColumn) ?
                                xx+_nWordBoxHighlightOffset :  //xx+(count/4) for n box blocks in boxes.png
                                (PG_PLAY == _state && _maxwordlen!=_longestWordLen && yy < _boxWordNeeded[xx]) ?
                                    xx+_nWordBoxNeededOffset :
                                    xx
                        )-3 );  //-3 to reset the xx back to 0 as the boxes.png starts at 3 letter tile
                }
			}
		}
		else
		{
		    //draw empty rect to denote no words of xx length at all
            //using sdlGfxPrimitives: rectangleRGBA(s->surface(), _boxOffset[xx], yo, _boxOffset[xx]+_boxLength[xx], yo+BOXH, 100, 100, 100, 100); //grey, faded
            if (_gd._mode <= GM_REWORD) //only ARCADE and REWORD modes have empty boxes drawn
                _gd._boxes.blitTo( s, _boxOffset[xx], yo, xx+(_nWordBoxEmptyOffset-3));	//tile 0=3, 1=4, 2=5, 3=6 etc. letter words
		}
	}	//for

	//play controls always sit on top of anything else
    _controlsPlay.render(s);

#if defined(_DEBUG)
    if (_dbg_display)
    {

        _gd._fntClean.put_text(s, 0, SCREEN_HEIGHT - _gd._fntClean.height(), _gd._words.getWord6().c_str(), BLUE_COLOUR);	//##DEBUG## shows first target word

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
	const int yyTitle = _gd._scorebar.tileH() + minGap;
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
		_gd._fntMed.put_number(s, yyReward, _bonusScore, "Just enough, add %d points", PURPLE_COLOUR, true);
		break;
	case SU_GOT6:		//WELL DONE! - You got the 6
		switch (_randomTitle)
		{
		case 0:_gd._fntBig.put_text(s, yyTitle, "GREAT!", PURPLE_COLOUR, true);break;
		case 1:_gd._fntBig.put_text(s, yyTitle, "GOOD!", PURPLE_COLOUR, true);break;
		case 2:_gd._fntBig.put_text(s, yyTitle, "COOL!", PURPLE_COLOUR, true);break;
		default:_gd._fntBig.put_text(s, yyTitle, "WELL DONE!", PURPLE_COLOUR, true);break;
		}
		_gd._fntMed.put_number(s, yyReward, _bonusScore, "You got a Reword, add %d points", PURPLE_COLOUR, true);
		break;
	case SU_BONUS:		//BONUS!! - You got all words
		switch (_randomTitle)
		{
		case 0:_gd._fntBig.put_text(s, yyTitle, "PERFECT!", PURPLE_COLOUR, true);break;
		case 1:_gd._fntBig.put_text(s, yyTitle, "EXCELLENT!", PURPLE_COLOUR, true);break;
		case 2:_gd._fntBig.put_text(s, yyTitle, "AMAZING!!", PURPLE_COLOUR, true);break;
		default:_gd._fntBig.put_text(s, yyTitle, "AWESOME!!", PURPLE_COLOUR, true);break;
		}
		_gd._fntMed.put_number(s, yyReward, _bonusScore, "All words, add %d points", PURPLE_COLOUR, true);
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

	case SU_SPEED6:		//Speed 6 mode - a single 6 letter word achieved
		if (_fastest)
		{
			_gd._fntBig.put_text(s, yyTitle, "FASTEST YET!", GOLD_COLOUR, true);
			_gd._fntMed.put_number(s, yyReward, _fastest, "You got it in %d seconds!", GOLD_COLOUR, false);
			_gd._fntMed.put_number(s, yyBonus, (maxCountdown()-_fastest)*SCORE_FASTEST, "Bonus: %d !", GOLD_COLOUR, false);
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
			_gd._fntMed.put_text(s, yyReward, "Now try another", GOLD_COLOUR, false);
		}
		break;
	case SU_TIMETRIAL:	//Time trial - single count down to 0 - no timer reset
		if (_fastest)
		{
			_gd._fntBig.put_text(s, yyTitle, "FASTEST YET!", BLUE_COLOUR, true);
			_gd._fntMed.put_number(s, yyReward, _fastest, "You got it in %d seconds!", BLUE_COLOUR, false);
			_gd._fntMed.put_number(s, yyBonus, (maxCountdown()-_fastest)*SCORE_FASTEST, "Bonus: %d !", BLUE_COLOUR, false);
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
			_gd._fntMed.put_text(s, yyReward, "get another one...", BLUE_COLOUR, false);
		}
		break;

	default:
	case SU_GAMEOVER:	//Game Over - You're a high scorer!
		_gd._fntBig.put_text(s, yyTitle, "Game Over", RED_COLOUR, true);
		_gd._fntMed.put_text(s, yyReward, "But you're a high scorer!", RED_COLOUR, false);
		break;
	}

	//helpful message - if a dict definition available tell player
	if (PG_END == _state)	//BODGE - only show if in end state(as render_wait() calls this), otherwise "Press B..." shows for a second
	{
        //put comment just above found words in boxes
		const int yo = (_yScratchBot+CURSORH+_boxOffsetY) - _gd._fntClean.height() - (_gd._fntClean.height()/2);
		_gd._fntClean.put_text(s,
			yo,	(_tmpDefMore) ? "Y for more detail, or B to continue" : "Press B to continue", GREY_COLOUR, false);
	}
}

void PlayGame::render_pause(Screen* s)
{
	const SDL_Colour c = BLACK_COLOUR;
	s->drawSolidRect(0,0,s->width(), s->height(), c);
	_roundPaused->draw(s);
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
	//Do repeat keys...
	//if a key is pressed and the interval has expired, process
	//that button as if pressesd again

    if (input->repeat(pp_i::UP))	button(input, pp_i::UP);
    if (input->repeat(pp_i::DOWN))  button(input, pp_i::DOWN);
    if (input->repeat(pp_i::LEFT))	button(input, pp_i::LEFT);
    if (input->repeat(pp_i::RIGHT)) button(input, pp_i::RIGHT);

	_round.work();
	_gd._word_last_pulse.work();
	_gd._word_totop_pulse.work();
	_gd._word_shuffle_pulse.work();
	_gd._word_try_pulse.work();

//  audio not animated icons so don't need to call work()
//	_gd._gamemusic_icon.work();

//    _controlsPlay.work(input, speedFactor);

}
void PlayGame::work_wait(Input* input, float speedFactor)
{
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
}
void PlayGame::work_pause(Input* input, float speedFactor)
{
	_roundPaused->work();
}
void PlayGame::work_popup(Input* input, float speedFactor)
{
	_pPopup->work(input, speedFactor);

	//and still allow the play controls to work during popup
//    _controlsPlay.work(input, speedFactor);
}

void PlayGame::startPopup(Input *input)
{
	_pPopup = new PlayGamePopup(_gd, foundEnoughWords());
	if (_pPopup)
		_pPopup->init(input);
}

void PlayGame::stopPopup()
{
	delete _pPopup;
	_pPopup = NULL;
}

void PlayGame::button(Input* input, pp_i::eButtonType b)
{
	//first handle any global keys
	switch (b)
	{
	case pp_i::L:
		_inputL = input->isPressed(b);
		break;
	case pp_i::R:
		_inputR = input->isPressed(b);
		break;
	case pp_i::SELECT:
		if (input->isPressed(b) && (_state != PG_PAUSE))
		{
			if (_pPopup)
				stopPopup();
			else
				startPopup(input);
		}
		break;
	case pp_i::CLICK:
		if (input->isPressed(b))
		{
			if (_inputL && _inputR)
			{
				_bAbort = true;
				pp_g::pushSDL_Event(USER_EV_END_COUNTDOWN); //try to exit more gracefully, pushes end of level
			}
			else
			{
				pp_g::pushSDL_Event(USER_EV_NEXT_TRACK);	//try to start next music track
			}
		}
		break;
	default:
		break;
	}

    if (_play)
    {
        _play->button(input, b);
        return;
    }

	//now handle popup menu on top of curr screen or depending on state,
	//call the function pointer of the correctly mapped button function
	if (_pPopup)
		button_popup(input, b);
	else
		(*this.*pButtonFn)(input, b);

}

void PlayGame::button_play(Input* input, pp_i::eButtonType b)
{
	if (!_waiting.done()) return;	//no user input until finished waiting

	//not waiting so allow button use...
	switch (b)
	{
	case pp_i::LEFT:
		if (input->isPressed(b)) _round.cursorPrev();
		break;
	case pp_i::RIGHT:
		if (input->isPressed(b)) _round.cursorNext();
		break;
	case pp_i::UP:
		if (input->isPressed(b)) _round.cursorUp();
		break;
	case pp_i::DOWN:
		if (input->isPressed(b)) _round.cursorDown(); //will only go down if letters exist
		break;
	case pp_i::L:
		if (input->isPressed(b)) commandWordToLast();
		break;
	case pp_i::R:
		if (input->isPressed(b)) commandWordToLast();
		break;
	case pp_i::A:
		if (input->isPressed(b))
		{
			commandJumbleWord();
		}
		break;
	case pp_i::B:
		if (input->isPressed(b))
		{
			//user still in play so select curr letter
			if (_round.cursorIsTop())
	  	 		_round.moveLetterDown();
			else
			{
	    		_round.moveLetterUp();
				//if no letters left on bottom row, go back to top
				if (!_round.cursorPrev()) _round.cursorUp();
			}
//			_round.cursorNext();	//dont use as it confuses player (well me anyway)
		}
		break;
	case pp_i::START:
		if (input->isPressed(b)) doPauseGame();
		break;
	case pp_i::Y:
		if (input->isPressed(b)) commandClearAllToTop();
		break;
	case pp_i::X:
		if (input->isPressed(b)) commandTryWord();
		break;

	default:break;
	}
}
void PlayGame::button_wait(Input* input, pp_i::eButtonType b)
{
	//same as play state
	button_play(input, b);
}
void PlayGame::button_end(Input* input, pp_i::eButtonType b)
{
	//in end state, not waiting so allow button use...
	switch (b)
	{
	case pp_i::B:
		if (input->isPressed(b))
			doMoveOn();
		break;
	case pp_i::LEFT:
		if (input->isPressed(b))
		{
			do {	//repeat jump left if finds a gap (missing 3, 4, 5 letter word)
				if (_xxWordHi-1 < _shortestWordLen) _xxWordHi = _longestWordLen; else --_xxWordHi;
			} while (_gd._words.wordsOfLength(_xxWordHi) == 0);
			if (_yyWordHi > _gd._words.wordsOfLength(_xxWordHi)-1) _yyWordHi = _gd._words.wordsOfLength(_xxWordHi)-1;
		}
		break;
	case pp_i::RIGHT:
		if (input->isPressed(b))
		{
			do {	//repeat jump right if finds a gap (missing 3, 4, 5 letter word)
				if (_xxWordHi+1 > _longestWordLen) _xxWordHi = _shortestWordLen; else ++_xxWordHi;
			} while (_gd._words.wordsOfLength(_xxWordHi) == 0);
			if (_yyWordHi > _gd._words.wordsOfLength(_xxWordHi)-1) _yyWordHi = _gd._words.wordsOfLength(_xxWordHi)-1;
		}
		break;
	case pp_i::UP:
		if (input->isPressed(b))
		{
			if (_yyWordHi-1 < 0) _yyWordHi = _gd._words.wordsOfLength(_xxWordHi)-1; else --_yyWordHi;
		}
		break;
	case pp_i::DOWN:
		if (input->isPressed(b))
		{
			if (_yyWordHi+1 > _gd._words.wordsOfLength(_xxWordHi)-1) _yyWordHi = 0; else ++_yyWordHi;
		}
		break;

	case pp_i::Y:
	case pp_i::CLICK:
//		if (!_bAbort && //make sure global L+R+Click not pressed
		if (input->isPressed(b))
		{
			doDictionary();		//toggle dictionary mode
		}
		break;

	default:break;
	}

}

void PlayGame::button_pause(Input* input, pp_i::eButtonType b)
{
	switch (b)
	{
	case pp_i::START:
	case pp_i::B:
		if (input->isPressed(b)) doPauseGame();	//will un-pause if it's in pause mode
		break;

	default:break;
	}
}
void PlayGame::button_popup(Input* input, pp_i::eButtonType b)
{
	_pPopup->button(input, b);
	handlePopup();
}
void PlayGame::handlePopup()
{
	if (!_pPopup->isSelected()) return;

	switch (_pPopup->selectedId())
	{
	case PlayGamePopup::POP_CANCEL:
		//continue with game level
		break;
	case PlayGamePopup::POP_SKIP:
		if (_state != PG_END)	//not already at end
			pp_g::pushSDL_Event(USER_EV_END_COUNTDOWN);	//next level - if not got a 6, will end game
		break;
	case PlayGamePopup::POP_SAVE:
		//user selected to save state (now at least a 6 letter word found)
		//and allow resume game later, so follow on to POP_QUIT
	case PlayGamePopup::POP_QUIT:
		//back to main menu - like L+R+Click (or ESC on PC)
		_bAbort = true;
		pp_g::pushSDL_Event(USER_EV_END_COUNTDOWN); //try to exit more gracefully, pushes end of level
		break;

	default:break;	//do nothing
	}

	stopPopup();	//deletes _pPopup after user selects, if SELECT pressed, button() fn stops popup
}

bool PlayGame::touch(const Point &pt)
{

    if (_play)
    {
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
	return true;
}
bool PlayGame::touch_play(const Point &pt)
{
    _controlsPlay.touched(pt);    //needed to highlight a touched control

	if (_round.cursorAt(pt))
	{
		if (_round.cursorIsTop())
        {
			_round.moveLetterDown();
			_round.cursorDown();
        }
		else
		{
			_round.moveLetterUp();
			if (!_round.cursorPrev()) _round.cursorUp();
		}
	}
	else if (_gd._word_last_pulse.contains(pt))
	{
		commandWordToLast();
	}
	else if (_gd._word_totop_pulse.contains(pt))
	{
		commandClearAllToTop();
	}
	else if (_gd._word_shuffle_pulse.contains(pt))
	{
		commandJumbleWord();
	}
	else if (_gd._word_try_pulse.contains(pt))
	{
		commandTryWord();
	}
    else if (_gd._gamemusic_icon.contains(pt))
    {
        _gd._options._bMusic = !_gd._options._bMusic;
        _gd._gamemusic_icon.setFrame(_gd._options._bMusic?0:1);    //first frame (on) or second frame (off)
        Locator::GetAudio().pushPauseTrack();
    }
    else if (_pause_rect.contains(pt))
    {
        doPauseGame();
    }
	else
		if (!_doubleClick.done())
		    commandTryWord();
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
		if (pt._x < _boxOffset[xx] || pt._x > _boxOffset[xx]+_boxLength[xx]) continue;

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
    if (_play != NULL)
    {
        return _play->tap(pt);
    }

    _ctrl_id = _controlsPlay.tapped(pt);

    if (_ctrl_id == CTRLID_MENU)
    {
        //need to push a key command instaed of starting as it needs an Input ptr
        int key = Locator::GetInput().un_translate(pp_i::SELECT);
        pp_g::pushSDL_EventKey(key);
        return true;
    }
    else if (_ctrl_id == CTRLID_NEXT || _ctrl_id == CTRLID_EXIT)
    {
        if (PG_END == _state)
        {
            _controlsPlay.enableControl(false, CTRLID_NEXT);
            _controlsPlay.enableControl(false, CTRLID_EXIT);
            doMoveOn();
            return true;
        }
    }

    return false;
}


//command issued by player - to place last word in scratch panel
void PlayGame::commandWordToLast()
{
	//start pulse anim and launch command
	_gd._word_last_pulse.startAnim(0, -1, ImageAnim::ANI_ONCE, 20);
	_round.setWordToLast();
}
//command issued by player - to place all chars in the top row
void PlayGame::commandClearAllToTop()
{
	_gd._word_totop_pulse.startAnim(0, -1, ImageAnim::ANI_ONCE, 20);
	_round.clearAllToTop();
}
//command issued by player - to jumble all remaining letters in the top row
void PlayGame::commandJumbleWord()
{
	_gd._word_shuffle_pulse.startAnim(0, -1, ImageAnim::ANI_ONCE, 20);
	if (_round.jumbleWord())
		Mix_PlayChannel(-1,_gd._fxWoosh,0);	//sound only if not already moving etc
}
//command issued by player - to check word selected against dictionary
void PlayGame::commandTryWord()
{
	_gd._word_try_pulse.startAnim(0, -1, ImageAnim::ANI_ONCE, 20);
	tryWord();
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
				Mix_PlayChannel(-1,_gd._fxCountdown,0);
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
					Mix_PlayChannel(-1,_gd._fxBonus,0);
					int bonus = SCORE_BONUS + (_countdown*SCORE_SECONDS);	//say: 100 + remaining seconds * 10
					_gd._score.addCurrScore(bonus);
					showSuccess(SU_BONUS, bonus);
				}
				else
				{
					//great! at least one 6 letter found so go to next level
					//If in SPEED6 or TIMETRIAL mode, only one 6 letter word needed to move on...

					//fill rest of found words list with words to be found and
					//display them in diff colour to show player what they missed
					fillRemainingWords();

					Mix_PlayChannel(-1,_gd._fx6found,0);

					if (_gd._mode <= GM_REWORD)
					{
					    //if arcade, set arcade bonus unless got a all-letter word, give reword bonus
					    int bonus = (_maxwordlen==_longestWordLen)?SCORE_BONUS:SCORE_ARCADE;
						_gd._score.addCurrScore(bonus);
						showSuccess((_maxwordlen==_longestWordLen)?SU_GOT6:SU_ARCADE, bonus);
					}
					else
					{
						//it's GM_SPEED6 or GM_TIMETRIAL so add score and any fastest bonus,
						//then move to next word
						_gd._score.addCurrScore(SCORE_WORD6);
						if (_fastest) _gd._score.addCurrScore((maxCountdown()-_fastest)*SCORE_FASTEST);
						if (GM_SPEED6 == _gd._mode)
						{
							showSuccess(SU_SPEED6);
						}
						else //GM_TIMETRIAL
						{
							showSuccess(SU_TIMETRIAL, _fastest);
						}
					}
				}
				_gd._score.addCurrWords(1);

                _controlsPlay.enableControl(true, CTRLID_MENU);
                _controlsPlay.enableControl(false, CTRLID_EXIT);

                _controlsPlay.enableControl(true, CTRLID_NEXT);
			}
			else
			{
				//oh dear, ran out of time (or player forced exit)

				//fill rest of found words with words to be found and
				//display them in diff colour to show player what they missed
				fillRemainingWords();

				if (_gd._score.isHiScore(_gd._mode, _gd._diffLevel) != -1)
				{
					//excellent! player got on scoreboard
					Mix_PlayChannel(-1,_gd._fx6notfound,1);	//play twice if got on hiscore
					showSuccess(SU_GAMEOVER);
				}
				else
				{
					//oh dear, ran out of time without a high enough score
					Mix_PlayChannel(-1,_gd._fx6notfound,2);	//play 3 times if no hiscore
					showSuccess(SU_BADLUCK);
				}

                _controlsPlay.enableControl(false, CTRLID_MENU);
                _controlsPlay.enableControl(true, CTRLID_EXIT);

                _controlsPlay.enableControl(false, CTRLID_NEXT);    //leave 0 countdown showing
			}
		}
		if (USER_EV_EXIT_SUB_SCREEN == sdlevent.user.code)
		{
		    //pop latest game state change (e.g PlayGameDict or PlayGamePause)
            statePop();
		}
	}
}

void PlayGame::doMoveOn()
{
	//play over so do we go to next level or back to menu...?
	switch (_success)
	{
    case SU_ARCADE:                 //Well done, you got enough words to continue
	case SU_GOT6:					//WELL DONE! - You got the 6
	case SU_SPEED6:					//or if in Speed6 mode, a 6 entered so move onto next level
	case SU_BONUS:					//BONUS!! - You got all words
		statePop(); 				//out of PG_END state
		newLevel();
		break;
	case SU_TIMETRIAL:				//in time trial mode quickly move on to next level immediately
		if (_countdown==0)
		{
			_bAbort = true;
			pp_g::pushSDL_Event(USER_EV_END_COUNTDOWN); //try to exit more gracefully, pushes end of level
		}
		else
		{
			statePop(); 				//out of PG_END state
			newLevel();
		}
		break;
	case SU_BADLUCK:				//Bad Luck! - Out of time
		exit(ST_MENU);
		break;

	default:
	case SU_GAMEOVER:				//Game Over - You're a high scorer!
		exit(ST_HIGHEDIT);
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

		tWordsFoundList::const_iterator it;
		it = _wordsFound[_xxWordHi].begin();
		for (int yy=0; yy<(int)_wordsFound[_xxWordHi].size(); ++yy)	//find the matching word in the list
		{
			if (_yyWordHi == yy && it != _wordsFound[_xxWordHi].end())
			{
				_dictWord = (*it)._word;
				break;
			}
			++it;
		}

		//display current highlighted word on seperate 'screen'
		statePush(PG_DICT); //_state = PG_DICT;
	}
}

void PlayGame::doPauseGame()
{
	//check if already paused
	if (PG_PAUSE == _state)
	{
		//is paused so unpause
		statePop(); //_state = _oldState;

		//penalty for pause, at least 10 seconds used
		if (_countdown > 10) _countdown-=10;

		//release the countdown timer
		startCountdown();
		return;
	}
	//else is not paused, so check if we can pause

	//if not in play mode dont need to pause the game
	if (PG_PLAY != _state) return;

	//only on easy mode, or at least one 6 letter word found on this level
	if (DIF_EASY == _gd._diffLevel || _wordsFound[_longestWordLen].size())
	{
		//pause the countdown timer
		stopCountdown();

		statePush(PG_PAUSE); //_state = PG_PAUSE;

		//prepare roundel class ready for a pause
		_roundPaused = std::auto_ptr<Roundels>(new Roundels());
		_roundPaused->setWordCenterHoriz(std::string("PAUSED"), _gd._letters,
										(SCREEN_HEIGHT/2)-_gd._letters.tileH(), 4);
		_roundPaused->startMoveFrom(Screen::width(), 0, 10, 50, 18, 0);

		Mix_FadeOutChannel(-1, 1000);
	}
}

void PlayGame::newGame()
{
	//Y pos of scratch area, doesn't change during game so calc here first
	//   (X pos depends on number of letters in word so calculated in newLevel()
	//scratch letter area _yScratchTop (6 roundels on top)
	//scratch box area _yScratchBot (6 boxes below roundels)
	_yScratchTop = _gd._scorebar.tileH() + GAME_GAP1;
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
	_countdownID = SDL_AddTimer(rate, countdown_callback, NULL);
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
	if (_gd._mode == GM_SPEED6)
		return ((int)DIF_MAX - _gd._diffLevel)*COUNTDOWN_SPEED6;
	//else its REWORD
	return ((int)DIF_MAX - _gd._diffLevel)*COUNTDOWN_REWORD;
//#else
//	return 30;	//##DEBUG##
//#endif
}

void PlayGame::newLevel()
{
	stopCountdown();
	_ctrl_id = 0;       //nothing pressed yet

	int xx;
	for (xx=0; xx<=TARGET_MAX; ++xx)
		_wordsFound[xx].clear();

	std::string newword;
	//nextWord() returns false if bad dictionary entry (XXXXXX corrupted or hacked) !
	bool bWord = _gd._words.nextWord(newword, _gd._diffLevel, _gd._mode);	//return next word found.
	_longestWordLen = newword.length();
	_shortestWordLen = _longestWordLen-(MAX_WORD_ROW-1); //say if longest is 6, then shortest is 3 (for 3, 4, 5, 6)

	//X pos of scratch area depends on length of word so calc here at each new level/word)
	_xScratch = (SCREEN_WIDTH - ((_longestWordLen * (CURSORW+2)) -2) ) /2;

	_boxOffsetY = 9;	//default y offset under scratch area - reset by speed6 and timetrial to be slightly lower

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
		else	//GM_SPEED6 or GM_TIMETRIAL
		{
			//current_w is not supported in my gp2x SDL build, so saved from video setup
//			const SDL_VideoInfo *pVI = SDL_GetVideoInfo();	//for pVI->current_w
			_boxOffset[n] = _boxLength[n] = 0;
			if (n == _longestWordLen)
			{
				//middle of screen as only a single N letter word column
				_boxOffset[n] = (_gd._current_w / 2) - (_gd._boxes.tileW() / 2);
				_boxLength[n] = xLen;

				_boxOffsetY = 30;	//slightly lower as it has only 1 box
			}
		}
	}

	_round.setWord(newword, _gd._letters, _xScratch+2, _yScratchTop+2, 6, true);
	_round.setTopAndBottomYPos(_yScratchTop+2, _yScratchBot+2);
	if (bWord)
		_round.jumbleWord(false);		//randomize the letters
	_round.startMoveFrom(Screen::width(), 0, 15, 100, 18, 0);//animate roundels into screen pos

	//paint this levels word boxes onto the background for quick blit display rather
	//than having to loop through and blit each box every frame as in prev versions
	prepareBackground();

	//if in REWORD or SPEED6 then reset the timer. TimeTrial keeps counting down...
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
	const int posLeft = 4;
	const int posRight = SCREEN_WIDTH-_gd._word_try_pulse.tileW()-4;	//all these touch incons same size
	const int posTop = _yScratchTop+2+((LETTERH-_gd._word_try_pulse.tileH())/2);	//all same size
	const int posBot = _yScratchBot+2+((LETTERH-_gd._word_try_pulse.tileH())/2);
	_gd._word_shuffle_pulse.setPos(posLeft, posTop);
	_gd._word_try_pulse.setPos(posLeft, posBot);
	_gd._word_totop_pulse.setPos(posRight, posTop);
	_gd._word_last_pulse.setPos(posRight, posBot);
	_gd._gamemusic_icon.setPos(_gamemusic_icon_x, _gamemusic_icon_y);

	//don't want to show or allow touch controls if not on a touchscreen (or mouse/PC) device
	_gd._word_shuffle_pulse.setVisible(_gd._bTouch);
	_gd._word_shuffle_pulse.setTouchable(_gd._bTouch);
	_gd._word_totop_pulse.setVisible(_gd._bTouch);
	_gd._word_totop_pulse.setTouchable(_gd._bTouch);
	_gd._word_last_pulse.setVisible(_gd._bTouch);
	_gd._word_last_pulse.setTouchable(_gd._bTouch);
	_gd._word_try_pulse.setVisible(_gd._bTouch);
	_gd._word_try_pulse.setTouchable(_gd._bTouch);

	_gd._word_last_pulse.setFrame(5);	//initially to last frame (button anim always goes back to last frame)
	_gd._word_totop_pulse.setFrame(5);
	_gd._word_shuffle_pulse.setFrame(5);
	_gd._word_try_pulse.setFrame(5);
	_gd._gamemusic_icon.setFrame(_gd._options._bMusic?0:1);    //first frame (on) or second frame (off)

    calcArcadeNeededWords();    //arcade mode highlights

	startCountdown();
	clearEventBuffer();	//start fresh each level
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
        //for med, need to find 50% of all words, or an all-letter word
        //for hard, need to find 75% of all words, or an all-letter word
        const int pc = (DIF_EASY == _gd._diffLevel)?25:(DIF_MED == _gd._diffLevel)?50:75;
        int needed = (nTotalLetters * ((float)pc/100));
_debugNeededAll = needed; //for debug display

        for (xx=_longestWordLen; xx>=_shortestWordLen; --xx)
        {
            //remove letters (in words) already found
            needed -= (int)_wordsFound[xx].size() * xx;
        }
_debugNeededNow = needed;

        for (xx=_longestWordLen; xx>=_shortestWordLen; --xx)
        {
            //only count nWords on screen, not overflow
            const int nWords = std::min(_gd._words.wordsOfLength(xx), MAX_WORD_COL);
            for (yy = 0; yy < nWords; ++yy)
            {
                if (needed >= xx)
                {
                    _boxWordNeeded[xx]++;
                    needed -= xx;
                    if (xx == _longestWordLen) break; //only need one longest word
                }
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
	int wordlen = tryWordAgainstDict();

	 //speed6 and timetrial only need a 6 to continue
	if (GM_TIMETRIAL == _gd._mode || GM_SPEED6 == _gd._mode)
	{
		if (_longestWordLen == wordlen)
		{
			pp_g::pushSDL_Event(USER_EV_END_COUNTDOWN);	//pushes end of level
			return;
		}
		else wordlen = -1;	//speed6 or timetrial badWord sound as < 6 letters
	}

	//must be GM_ARCADE or GM_REWORD or bad word
	if (wordlen >= _shortestWordLen && wordlen <= _longestWordLen)	//is 3,4,5, or 6 (or whatever)
	{
		_gd._score.addCurrScore( (_longestWordLen == wordlen)?SCORE_WORD6:SCORE_WORD );	//more for a 6 letter word
		_round.clearAllToTop(true);	//remove hit word

		if (foundAllWords())
		{
			pp_g::pushSDL_Event(USER_EV_END_COUNTDOWN); //pushes end of level
			return;	//exit before sound, as success() plays fanfare sound
		}
		//it's a simple word find
		Mix_PlayChannel(-1,(_longestWordLen == wordlen)?_gd._fx6found:_gd._fxFound,0);

		calcArcadeNeededWords();
	}
	else	//0=already found, -1=not a 6 or in sub word list
	{
		Mix_PlayChannel(-1,(0 == wordlen)?_gd._fxOldword:_gd._fxBadword,0);
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
        int xx, iCount(0);
        for (xx=_shortestWordLen; xx<=_longestWordLen; iCount += _boxWordNeeded[xx++]);
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
	_gamebg = std::auto_ptr<Image>(new Image(SCREEN_WIDTH, SCREEN_HEIGHT));
	_gamebg->drawSolidRect(0, 0, Screen::width(), Screen::height(), GAMEBG_COLOUR);

	//place score bar centered - in case screen bigger than graphic
	int sb_x = (SCREEN_WIDTH - _gd._scorebar.width())/2;  //in case sb w < screen w
	int sb_w = _gd._scorebar.width();
	int sb_h = _gd._scorebar.height();
	_gamebg->blitFrom(&_gd._scorebar, -1, sb_x, 0);

	//prerender the score and words titles
	//find out sizes and calc reasonable positions
	FontTTF &fontText = _gd._fntSmall;
	FontTTF &fontNumbers = _gd._fntSmall;
	FontTTF &fontCounter = _gd._fntBig;
	Rect r(0, 0, 0, 0);
	int score_len(0), score0_len(0), words_len(0), words0_len(0), count0_len(0);
	r = fontText.calc_text_metrics("SCORE: ");		//note gap to look better
	score_len = r._max._x;
	r = fontNumbers.calc_text_metrics("00000000");	//8 0's not drawn here
	score0_len = r._max._x;
	r = fontText.calc_text_metrics("WORDS: ");		//note gap to look better
	words_len = r._max._x;
	r = fontNumbers.calc_text_metrics("0000");	//4 0's not drawn here
	words0_len = r._max._x;
	r = fontCounter.calc_text_metrics("000");	//countdown timer numbers (3 0's not drawn here)
	count0_len = r._max._x;

	//now calc where we can place these and the gap between each
    //
    // |-------------------------------------------------------------------------------------|
    // |                                                                       /------------\|
    // |   [  MENU  ]            SCORE:00000000           WORDS:0000           |    999     ||
    // |                                                                       \------------/|
    // |-------------------------------------------------------------------------------------|
    //  <  edge_pad ><equal_gap>               <equal_gap>          <equal_gap><  edge_pad  >
    //                           ^score_x                 ^words_x
    //       ^_menu_icon_x             ^_score0_x               ^_words0_x           ^_countdown0_x
    //

    int middle_len = score_len+score0_len + words_len+words0_len;
    int edge_pad = sb_x + (count0_len * 1.2);  //space taken up by [ MENU ] or [ 999 ]
	int equal_gap = (sb_w - middle_len - (edge_pad*2)) / 3;    //space on bar len after edges removed, /3 for equal dist

	int titles_y = ((sb_h - fontText.height()) / 2) + 2;	//+2 magic number - too high otherwise...?
	int numbers_y = ((sb_h - fontNumbers.height()) / 2) + 2;	//+2 magic number - too high otherwise...?

	int score_x = sb_x+edge_pad+equal_gap;          //x pos for "SCORE:"
	_score0_x = score_x+score_len;                  //x pos for score "00000000"
	_score0_y = numbers_y;

	int words_x = _score0_x+score0_len+equal_gap;   //x pos for "WORDS:"
	_words0_x = words_x+words_len;                  //x pos for words "0000"
	_words0_y = numbers_y;

	_countdown0_x = (sb_x + sb_w) - (edge_pad / 2);
	_countdown0_y = (sb_h - fontCounter.height()) / 2;

    int gameicon_y = (sb_h - _gd._gamemusic_icon.tileH()) / 2;
    _gamemusic_icon_x = edge_pad + (equal_gap / 5);
    _gamemusic_icon_y = gameicon_y;

	fontText.put_text(_gamebg.get(), score_x, titles_y, "SCORE:", WHITE_COLOUR);
	fontText.put_text(_gamebg.get(), words_x, titles_y, "WORDS:", WHITE_COLOUR);

	//draw mode in bot right corner (before/under word boxes so if 8 6-letter words, it doesnt cover anything up)
	Image *pImage = 0;
	switch (_gd._mode)
	{
	case GM_ARCADE:		pImage = &_gd._game_arcade;	break;  //##TODO
	case GM_REWORD:		pImage = &_gd._game_reword;	break;
	case GM_SPEED6:		pImage = &_gd._game_speed6;	break;
	case GM_TIMETRIAL:	pImage = &_gd._game_timetrial; break;
	default:break;
	}
	assert(pImage);
	int mode_x = SCREEN_WIDTH - pImage->width() + 25; 	//slightly off screen
	int mode_y = SCREEN_HEIGHT - pImage->height() + 25;	//ditto
	_gamebg->blitFrom(pImage, -1, mode_x, mode_y);

	//draw difficulty level in bot right corner (before/under word boxes so if 8 6-letter words, it doesnt cover anything up)
	_gd._fntSmall.put_text_right(_gamebg.get(), SCREEN_HEIGHT - _gd._fntSmall.height(), 0, _gd._diffName.c_str(), _gd._diffColour);
}


