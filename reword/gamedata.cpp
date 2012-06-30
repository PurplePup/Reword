////////////////////////////////////////////////////////////////////
/*

File:			gamedata.cpp

Class impl:		GameData

Description:	A wrapper class for all shared game data passed about in-game

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4.0	25.02.2008	Added game modes - "reword" "speeder" and "time trial"
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
#include "helpers.h"

#include <iostream>

GameData::GameData() : _bTouch(false),  _init(false)
{
}

GameData::GameData(GameOptions &opt) : _options(opt), _bTouch(false),  _init(false)
{
}

void GameData::init()
{
	//initialise, load everything needed...
	bool bErr = false;

	//SCORES ETC - load scores for this wordfile
    Uint32 hash = _score.loadUsingWordfileName(_options._defaultWordFile);

	//LOAD WORDS - 	pass score hash + ticks as random seed
	bErr |= !_words.load(RES_WORDS + _options._defaultWordFile, hash + SDL_GetTicks());

	//FONTS
	bErr |= !_fntTiny.load(RES_FONTS + "FreeSansBold.ttf", FONT_TINY);
	bErr |= !_fntClean.load(RES_FONTS + "FreeSansBold.ttf", FONT_SMALL);
	bErr |= !_fntSmall.load(RES_FONTS + "BD_Cartoon_Shout.ttf", FONT_SMALL);
	bErr |= !_fntMed.load(RES_FONTS + "BD_Cartoon_Shout.ttf", FONT_MEDIUM);
	bErr |= !_fntBig.load(RES_FONTS + "BD_Cartoon_Shout.ttf", FONT_BIG);

    if (!bErr)
    {
        //SOUNDS - no wrapper class so need to free on exit
        _fxCountdown = Mix_LoadWAV(std::string(RES_SOUNDS + "ping.wav").c_str());	//<10 seconds remaining
        _fxBadword = Mix_LoadWAV(std::string(RES_SOUNDS + "boing.wav").c_str());	//word not in dict
        _fxOldword = Mix_LoadWAV(std::string(RES_SOUNDS + "beepold.wav").c_str());	//word already picked
        _fx6notfound = Mix_LoadWAV(std::string(RES_SOUNDS + "honk.wav").c_str());	//not found a 6 letter word
        _fx6found = Mix_LoadWAV(std::string(RES_SOUNDS + "binkbink.wav").c_str());	//found a/the 6 letter word
        _fxFound = Mix_LoadWAV(std::string(RES_SOUNDS + "blipper.wav").c_str());	//found a non 6 letter word
        _fxBonus = Mix_LoadWAV(std::string(RES_SOUNDS + "fanfare.wav").c_str());	//all words done before countdown
        _fxWoosh = Mix_LoadWAV(std::string(RES_SOUNDS + "woosh2.wav").c_str());		//jumble letters sound
        _fxRoundel = Mix_LoadWAV(std::string(RES_SOUNDS + "blipper.wav").c_str());	//roundel press
        _fxControl = Mix_LoadWAV(std::string(RES_SOUNDS + "blipper.wav").c_str());	//control press

        //#ifdef _USE_MIKMOD
        _musicMenu = Mix_LoadMUS(std::string(RES_SOUNDS + "cascade.mod").c_str());	//in sounds, not music dir
        std::cerr << "Mix_LoadMUS cascade.mod result : " << Mix_GetError() << std::endl;
        //#else
        //	_musicMenu = 0;	//load later
        //#endif
    }

	//GAME STATES
	//start in main menu
	_state = ST_MENU;
	_mainmenuoption = 0;	//start on [play] menu option
	//default to saved or medium diff
	setDiffLevel(_options._defaultDifficulty);
	//default to "classic" game mode
	_mode = GM_ARCADE;

	//finally, are we done? If bErr was set true anywhere above then don't set _init true
	_init = !bErr;
}

GameData::~GameData()
{
	//other specific resources to clear up
    if (_init)
    {
        Mix_FreeChunk(_fxBadword);
        Mix_FreeChunk(_fxOldword);
        Mix_FreeChunk(_fxCountdown);
        Mix_FreeChunk(_fx6notfound);
        Mix_FreeChunk(_fx6found);
        Mix_FreeChunk(_fxFound);
        Mix_FreeChunk(_fxBonus);
        Mix_FreeChunk(_fxWoosh);
        Mix_FreeChunk(_fxRoundel);
        Mix_FreeChunk(_fxControl);

        Mix_FreeMusic(_musicMenu);
    }
}

//set the relevant vars (value, name, colour) for the difficulty level
void GameData::setDiffLevel(eGameDiff newDiff)
{
	_diffLevel = newDiff;
	_diffName = (_diffLevel==DIF_EASY)?"EASY":(_diffLevel==DIF_MED)?"MEDIUM":"HARD";
	_diffColour = (_diffLevel==DIF_EASY)?GREEN_COLOUR:(_diffLevel==DIF_MED)?ORANGE_COLOUR:RED_COLOUR;
}

//write/overwrite quickstate save file
void GameData::saveQuickState()
{
	tQuickStateSave	qss;
	qss._wordfile = _options._defaultWordFile;
	qss._words = _score.currWords();
	qss._score =  _score.currScore();
	qss._diff = (int)_diffLevel;
	qss._mode = (int)_mode;
	qss._seed = _score.seed();
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

		_score.setCurrWords(qss._words);
		_score.setCurrScore(qss._score);
		_diffLevel = (eGameDiff)qss._diff;
		_mode = (eGameMode)qss._mode;
		_score.setSeed(qss._seed);

		//load same word file as last load, same seed, next word on
		if (!_words.load(RES_WORDS + qss._wordfile, qss._seed, qss._words+1))
        {
            std::cerr << "Fall back to default word file" << std::endl;
            if (!_words.load(RES_WORDS + _options._defaultWordFile, SDL_GetTicks()))
            {
                //user won't see much without a word file - don't know how this'll end...
                std::cerr << "Cannot load default word file" << std::endl;
                return false;
            }
        }

		qs.quickStateDelete();	//remove last quick save file
		return true;
	}
	return false;
}


GameOptions::GameOptions() :
    _bSound(true), _bMute(false),
    _bSingleTapMenus(true), _bDefaultSfxOn(true), _bDefaultMusicOn(true),
    _defaultDifficulty(DIF_MED),   //medium diff
    _bDirty(false)
{
    _defaultWordFile = "eng_british_oxford.txt"; //"words/" gets prepended
    _defaultMusicDir = "music/";
#ifdef PANDORA
	//pandora .pnd directory structure requires a unnamed directory for data written back to the SD card,
	//so create a options directory in the startup/run .sh script and read/write scores files in there.
	_optionsFile = "./options.dat";
#else
	_optionsFile = RES_BASE + "options.dat";
#endif
}

GameOptions::~GameOptions()
{
    save();
}

bool GameOptions::load()
{
	std::string line, key, value;
	std::ifstream infile (_optionsFile.c_str(), std::ios_base::in);
	if (infile.is_open())
	{
		while (std::getline(infile, line, '\n'))
		{
			pptxt::splitKeyValuePair(line, key, value);
			if (key == "singletap")
				_bSingleTapMenus = atoi(value.c_str());
			else if (key == "defaultsfx")
				_bDefaultSfxOn = atoi(value.c_str());
			else if (key == "defaultmusic")
				_bDefaultMusicOn = atoi(value.c_str());
			else if (key == "defaultdiff")
			{
			    int diff = atoi(value.c_str());
			    if (diff > (int)DIF_UNDEFINED && diff < (int)DIF_MAX)
                    _defaultDifficulty = (eGameDiff)diff;
			}
			else if (key == "defaultmusicdir")
				_defaultMusicDir = value.c_str();
			else if (key == "defaultwordfile")
				_defaultWordFile = value.c_str();
		}
		_bDirty = false;
		return true;
	}
	_bDirty = true; //force save if options file not found
	return false;
}

bool GameOptions::save()
{
    if (!_bDirty) return false;
	//save options as simple text pairs
 	std::ofstream outfile(_optionsFile.c_str(), std::ios::out|std::ifstream::trunc);
	if (outfile.is_open())
	{
		outfile << "Reword=" << VERSION_STRING << std::endl;
		outfile << "singletap=" << _bSingleTapMenus << std::endl;
		outfile << "defaultsfx=" << _bDefaultSfxOn << std::endl;
		outfile << "defaultmusic=" << _bDefaultMusicOn << std::endl;
		outfile << "defaultdiff=" << _defaultDifficulty << std::endl;
		outfile << "defaultmusicdir=" << _defaultMusicDir << std::endl;
		outfile << "defaultwordfile=" << _defaultWordFile << std::endl;
	}
	_bDirty = false;
    return true;
}

bool GameOptions::setDefaultWordFile(const std::string &wordFile)
{
    if (_defaultWordFile!= wordFile)
    {
        _bDirty = true;
        _defaultWordFile = wordFile;
    }
    return _bDirty;
}

//make sure dirty flag set
void GameOptions::setSingleTap(bool b)
{
    if (_bSingleTapMenus != b) _bDirty = true;
    _bSingleTapMenus = b;
}

//make sure dirty flag set
void GameOptions::setDefaultDiff(eGameDiff e)
{
    if (_defaultDifficulty != e) _bDirty = true;
    _defaultDifficulty = e;
}

//make sure dirty flag set
void GameOptions::setDefaultSfxOn(bool b)
{
    if (_bDefaultSfxOn != b) _bDirty = true;
    _bDefaultSfxOn = b;
}

//make sure dirty flag set
void GameOptions::setDefaultMusicOn(bool b)
{
    if (_bDefaultMusicOn != b) _bDirty = true;
    _bDefaultMusicOn = b;
}
