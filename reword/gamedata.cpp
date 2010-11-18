////////////////////////////////////////////////////////////////////
/*

File:			gamedata.cpp

Class impl:		GameData

Description:	A wrapper class for all shared game data passed about in-game

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4.0	25.02.2008	Added game modes - "reword" "speed6" and "time trial"
 				0.5.0	18.06.2008	Added touch support and some animated touch icons
 				0.5.1	07.10.2008	seperate some gfx to allow diff screen sizes

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
#include "gamedata.h"
#include "platform.h"
#include "score.h"

#include <iostream>

GameData::GameData() : _bTouch(false),  _init(false)
{
	//initialise, load everything needed...
	bool bErr = false;

	//FONTS
	bErr |= !_fntTiny.load(RES_BASE + "fonts/FreeSansBold.ttf", FONT_TINY);
	bErr |= !_fntClean.load(RES_BASE + "fonts/FreeSansBold.ttf", FONT_SMALL);
	bErr |= !_fntSmall.load(RES_BASE + "fonts/BD_Cartoon_Shout.ttf", FONT_SMALL);
	bErr |= !_fntMed.load(RES_BASE + "fonts/BD_Cartoon_Shout.ttf", FONT_MEDIUM);
	bErr |= !_fntBig.load(RES_BASE + "fonts/BD_Cartoon_Shout.ttf", FONT_BIG);

	//BACKGROUNDS & IMAGES

	bErr |= !_boxes.load(RES_BASE + "images/boxes.png");
	bErr |= !_boxes.setTileSize(BOXW, BOXH);	//tile 0=3, 1=4, 2=5, 3=6 letter words

	bErr |= !_menubg.load(RES_BASE + "images/menubg.png");		//solid background (no alpha)
	bErr |= !_menubg_plain.load(RES_BASE + "images/menubg_plain.png");
	bErr |= !_menu_reword.load(RES_BASE + "images/menu_reword.png");
	bErr |= !_menu_speed6.load(RES_BASE + "images/menu_speed6.png");
	bErr |= !_menu_timetrial.load(RES_BASE + "images/menu_timetrial.png");
	bErr |= !_scorebar.load(RES_BASE + "images/scorebar.png");
	bErr |= !_game_reword.load(RES_BASE + "images/game_reword.png");
	bErr |= !_game_speed6.load(RES_BASE + "images/game_speed6.png");
	bErr |= !_game_timetrial.load(RES_BASE + "images/game_timetrial.png");
	bErr |= !_popupmenu.load(RES_BASE + "images/popup_menu.png", 200);
	bErr |= !_cursor.load(RES_BASE + "images/cursors.png");
	bErr |= !_cursor.setTileSize(CURSORW,CURSORH);
	bErr |= !_letters.load(RES_BASE + "images/roundel_letters.png", 255);
	bErr |= !_letters.setTileSize(LETTERW,LETTERH);

	bErr |= !_scratch.load(RES_BASE + "images/scratch_2.png", -1, 7);

	bErr |= !_word_last_pulse.load(RES_BASE + "images/btn_word_last.png", -1, 6);
	bErr |= !_word_totop_pulse.load(RES_BASE + "images/btn_word_totop.png", -1, 6);
	bErr |= !_word_shuffle_pulse.load(RES_BASE + "images/btn_word_shuffle.png", -1, 6);
	bErr |= !_word_try_pulse.load(RES_BASE + "images/btn_word_try.png", -1, 6);

	//SPRITES
	bErr |= !_arrowUp.load(RES_BASE + "images/btn_scroll_up.png", 255, 3);
	bErr |= !_arrowDown.load(RES_BASE + "images/btn_scroll_down.png", 255, 3);
	bErr |= !_arrowLeft.load(RES_BASE + "images/btn_scroll_left.png", 255, 3);
	bErr |= !_arrowRight.load(RES_BASE + "images/btn_scroll_right.png", 255, 3);
	bErr |= !_star.load(RES_BASE + "images/star.png", 255, 7);

//	bErr |= !_moreWords.load(RES_BASE + "images/morewords.png",255, 2);	//frame 1=more_to_get, 2=more_all_got

	//SOUNDS - no wrapper class so need to free on exit
	_fxCountdown = Mix_LoadWAV(std::string(RES_BASE + "sounds/ping.wav").c_str());	//<10 seconds remaining
	_fxBadword = Mix_LoadWAV(std::string(RES_BASE + "sounds/boing.wav").c_str());	//word not in dict
	_fxOldword = Mix_LoadWAV(std::string(RES_BASE + "sounds/beepold.wav").c_str());	//word already picked
	_fx6notfound = Mix_LoadWAV(std::string(RES_BASE + "sounds/honk.wav").c_str());	//not found a 6 letter word
	_fx6found = Mix_LoadWAV(std::string(RES_BASE + "sounds/binkbink.wav").c_str());	//found a/the 6 letter word
	_fxFound = Mix_LoadWAV(std::string(RES_BASE + "sounds/blipper.wav").c_str());	//found a non 6 letter word
	_fxBonus = Mix_LoadWAV(std::string(RES_BASE + "sounds/fanfare.wav").c_str());	//all words done before countdown
	_fxWoosh = Mix_LoadWAV(std::string(RES_BASE + "sounds/woosh2.wav").c_str());	//jumble letters sound

//#ifdef _USE_MIKMOD
	_musicMenu = Mix_LoadMUS(std::string(RES_BASE + "sounds/cascade.mod").c_str());	//in sounds, not music dir
	std::cerr << "Mix_LoadMUS cascade.mod result : " << Mix_GetError() << std::endl;
//#else
//	_musicMenu = 0;	//load later
//#endif

	//SCORES ETC
	Uint32 hash = _score.load();

	//LOAD WORDS - 	//pass score hash + ticks as random seed
	bErr |= !_words.load(RES_BASE + "words/rewordlist.txt", hash + SDL_GetTicks());

	//GAME STATES
	//start in main menu
	_state = ST_MENU;
	_mainmenuoption = 0;	//start on [play] menu option
	//default to medium diff
	setDiffLevel(DIF_MED);
	//default to "classic" game mode
	_mode = GM_REWORD;

	//finally, are we done? If bErr was set true anywhere above then don't set _init true
	_init = !bErr;
}

GameData::~GameData()
{
	//other specific resources to clear up

	Mix_FreeChunk(_fxBadword);
	Mix_FreeChunk(_fxOldword);
	Mix_FreeChunk(_fxCountdown);
	Mix_FreeChunk(_fx6notfound);
	Mix_FreeChunk(_fx6found);
	Mix_FreeChunk(_fxFound);
	Mix_FreeChunk(_fxBonus);

	Mix_FreeMusic(_musicMenu);
}

//set the relevant vars (value, name, colour) for the difficulty level
void GameData::setDiffLevel(eGameDiff newDiff)
{
	_diffLevel = newDiff;
	_diffName = (_diffLevel==DIF_EASY)?"   Easy":(_diffLevel==DIF_MED)?"Medium":"   Hard";
	_diffColour = (_diffLevel==DIF_EASY)?GREEN_COLOUR:(_diffLevel==DIF_MED)?ORANGE_COLOUR:RED_COLOUR;
}

void GameData::saveQuickState()
{
	tQuickStateSave	qss;
	qss.words = _score.currWords();
	qss.score =  _score.currScore();
	qss.diff = (int)_diffLevel;
	qss.mode = (int)_mode;
	qss.seed = _score.seed();
	QuickState qs;
	qs.setQuickState(qss);
	qs.quickStateSave();
}

//restore the last quick save state and allow resume playing at the next word
//in the word list (using the same random gen seed, so the list will be the
//same 'random' order).
bool GameData::loadQuickState()
{
	QuickState qs;
	if (qs.quickStateLoad())
	{
		tQuickStateSave	qss;
		qs.getQuickState(qss);

		_score.setCurrWords(qss.words);
		_score.setCurrScore(qss.score);
		_diffLevel = (eGameDiff)qss.diff;
		_mode = (eGameMode)qss.mode;
		_score.setSeed(qss.seed);

		//load same word file as last load, same seed, next word on
		_words.load("", qss.seed, qss.words+1);
		qs.quickStateDelete();	//remove last quick save file
		return true;
	}
	return false;
}

