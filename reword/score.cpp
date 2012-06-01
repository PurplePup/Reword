////////////////////////////////////////////////////////////////////
/*

File:			score.cpp

Class impl:		Score

Description:	A class to handle all aspects of loading/saving and managing scores

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.4		03.03.2008	New format score file incl. speeder & TimeTrial
				0.6     01.12.2011  Add Arcade mode scoring

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
#include "score.h"
#include "helpers.h"

#include <iostream>
#include <cerrno>

Score::Score()
{
#ifdef PANDORA
	//pandora .pnd directory structure requires a unnamed directory for data written back to the SD card,
	//so create a score directory in the startup/run .sh script and read/write scores files in there.
	_scorefile = "./hiscore.dat";
#else
	_scorefile = RES_BASE + "hiscore.dat";
#endif
	_seed = 0;
}

void Score::init()
{
	memset(&_hiScore[0], 0, sizeof(_hiScore[0])*3);
	memset(&_hiScoreS6[0], 0, sizeof(_hiScoreS6[0])*3);
	memset(&_hiScoreTT[0], 0, sizeof(_hiScoreTT[0])*3);
	memset(&_hiScoreRA[0], 0, sizeof(_hiScoreRA[0])*3);

	int item, diff;
	for (diff=0; diff<3; ++diff)
	{
		for (item=0; item<10; ++item)
		{
			memcpy(_hiScore[diff].level[item].inits, "---\0", 4);
			memcpy(_hiScoreS6[diff].level[item].inits, "---\0", 4);
			memcpy(_hiScoreTT[diff].level[item].inits, "---\0", 4);
			memcpy(_hiScoreRA[diff].level[item].inits, "---\0", 4);
		}
	}

	setCurrInits("AAA");	//needed for first time into hiscore update
}


//return a hash total to help seed random number gen
//NOTE: from v0.4, save files lead with a zero byte and a version byte to allow future score
// 		file mods without the player having to trash the score file and restart from scratch
Uint32 Score::load(std::string scorefile)
{
	if (!scorefile.length())
		scorefile = _scorefile;	//none passed in so load from default file
	else
		_scorefile = scorefile;	//file passed in so save as default

	init();

	//We add up byte values making up the score table to create the value used in
	//seeding the random number generator. Do it this way to get a slightly
	//different value each time the high score changes (as thats about the only
	//consistently changing persisted value we can use). I believe there isnt a
	//variable enough time clock on the gp2x to use for this purpose.

	unsigned int total = 0, diff = 0, item = 0;
	std::ifstream in(scorefile.c_str());

	if (in.is_open())
	{
		char version = 0, format = 0;
		version = in.get();
		if (in.bad()) return 0; 	//can't read first char - poss empty file
		if (0 == version)
		{
			//First byte is a zero (so is a v0.4 type score file) not an initial.
			//Score file versioning started in v0.4. Before that it was just a
			//straight dump of all 3 _hiScore entries (see old code in else block)
			format = in.get();
			if (in.bad()) return 0; 	//can't read version char - poss corrupt file
			if (format >= 0x01)		//first generation format
			{
				//load a version 0.4 save file
				for (diff=0; diff<3; ++diff)	//3 levels easy, med, hard
					in.read(reinterpret_cast<char*>(&_hiScore[diff]), sizeof(_hiScore[0]));
				//from v0.4, speeder and TimeTrial included in scores
				for (diff=0; diff<3; ++diff)
					in.read(reinterpret_cast<char*>(&_hiScoreS6[diff]), sizeof(_hiScoreS6[0]));
				for (diff=0; diff<3; ++diff)
					in.read(reinterpret_cast<char*>(&_hiScoreTT[diff]), sizeof(_hiScoreTT[0]));
			}
			if (format >= 0x02)		//2nd generation format
			{
				//load arade scores
				for (diff=0; diff<3; ++diff)	//3 levels easy, med, hard
					in.read(reinterpret_cast<char*>(&_hiScoreRA[diff]), sizeof(_hiScoreRA[0]));
			}
			//else - no other versions needed yet
		}
		else
		{
			//is an old style (pre v0.4) score file so load it in as before.
			//On saving, it will be output as the latest format, along with a version byte
			in.seekg (0, std::ios::beg);	//back to start
			tHiScoreLevelsPre04 hiScorePre04;
			for (diff=0; diff<3; ++diff)	//3 levels easy, med, hard
			{
				memset(&hiScorePre04, 0, sizeof(hiScorePre04));
				in.read(reinterpret_cast<char*>(&hiScorePre04), sizeof(hiScorePre04));
				//copy old format to new format
				for (item=0; item<10; ++item)
				{
					memcpy(_hiScore[diff].level[item].inits, hiScorePre04.level[item].inits, 4);
					_hiScore[diff].level[item].words = hiScorePre04.level[item].words;
					_hiScore[diff].level[item].score = hiScorePre04.level[item].score;
					_hiScore[diff].level[item].fastest = 0;
				}
			}
		}
		//add up loaded bytes
		int tpos = 0;
		unsigned char *p;
		for (tpos=0, p=(unsigned char*)&_hiScore[0]; tpos < (int)sizeof(_hiScore[0])*3; ++tpos) { total += (unsigned int)*p; ++p; }
		for (tpos=0, p=(unsigned char*)&_hiScoreS6[0]; tpos < (int)sizeof(_hiScoreS6[0])*3; ++tpos) { total += (unsigned int)*p; ++p; }
		for (tpos=0, p=(unsigned char*)&_hiScoreTT[0]; tpos < (int)sizeof(_hiScoreTT[0])*3; ++tpos) { total += (unsigned int)*p; ++p; }
		for (tpos=0, p=(unsigned char*)&_hiScoreRA[0]; tpos < (int)sizeof(_hiScoreRA[0])*3; ++tpos) { total += (unsigned int)*p; ++p; }
	}
	_seed = total;
	return total;
}

void Score::save(std::string scorefile)
{
	if (!scorefile.length()) scorefile = _scorefile; //none passed in so use default

	std::ofstream out(scorefile.c_str(), std::ios::out|std::ifstream::binary);
	if (out)
	{
        out.put( 0x00 );	//format: 0x00 byte first denotes >= v0.4 score file
        out.put( 0x02 );	//version: 0x02 denotes 2nd generation format

        int diff;
        for (diff=0; diff<3; ++diff)	//3 levels easy, med, hard
            out.write(reinterpret_cast<char*>(&_hiScore[diff]), sizeof(_hiScore[0]));		//Reword

        //from v0.4, speeder and TimeTrial included in scores
        for (diff=0; diff<3; ++diff)
            out.write(reinterpret_cast<char*>(&_hiScoreS6[diff]), sizeof(_hiScoreS6[0]));	//Speeder
        for (diff=0; diff<3; ++diff)
            out.write(reinterpret_cast<char*>(&_hiScoreTT[diff]), sizeof(_hiScoreTT[0]));	//TimeTrial

        //from v0.6, arcade mode
        for (diff=0; diff<3; ++diff)
            out.write(reinterpret_cast<char*>(&_hiScoreRA[diff]), sizeof(_hiScoreRA[0]));	//Arcade
	}
	else
	{
		std::cerr << "Cannot save score file: " << scorefile << " - error: " << strerror(errno) << std::endl;
	}
}

void Score::setCurrInits(std::string inits)
{
	//only use first 3 chars and make sure 4th is NULL
    if (inits.empty())
        inits = "AAA";
	memcpy(&_curr.inits, inits.c_str(), 3);
	_curr.inits[3] = '\0';

}
//set number of words to a value
void Score::setCurrWords(unsigned int words)
{
	 _curr.words = words;
}
//add to words
unsigned int Score::addCurrWords(unsigned int words)
{
	return _curr.words += words;
}
//set score to a value
void Score::setCurrScore(unsigned int score)
{
	 _curr.score = score;
}
//add to score
unsigned int Score::addCurrScore(unsigned int score)
{
	return _curr.score += score;
}
//set the current fastest value if it actually is faster, otherwise return false
bool Score::setCurrFastest(unsigned int fastest)
{
	if (_curr.fastest && (fastest >= _curr.fastest)) return false;
	_curr.fastest = fastest;
	return true;
}
void Score::resetCurr()
{
	setCurrWords(0);
	setCurrScore(0);
	setCurrFastest(0);
}

int Score::isHiScore(int mode, int diff)
{
	if (!_curr.score) return -1;	//never on hi score if scored zero

	tHiScoreLevels *pScore = getLevel(mode, diff);
	int item;
	for (item=0; item<10; ++item)	//from [0] top score to [9] lowest hi-score
		if ((_curr.score == pScore->level[item].score &&
			_curr.words > pScore->level[item].words ) ||
			_curr.score > pScore->level[item].score)
			return item;
/*
e.g.
	XXX		10382	(10)	--higher score lower #words
	XXX		10372	(11)	--lower score, higher words dont count (just took player longer!)
	XXX		10362	(11)
	XXX		10232	(11)
	ABC		10232	(10)
	XXX		10232	(10)	--scored after ABC so lower, must beat score to knock ABC off
	XXX		10232	(09)
	BBA		00170	(01)
*/
	return -1;
}

//move everything at and below the new hi sore pos down
//and drop off the last entry (9)
void Score::insert(int mode, int diff, int level, tHiScoreEntry &curr)
{
	tHiScoreLevels *pScore = getLevel(mode, diff);
	int item;
	for (item=9; item>=level; --item)
		memcpy(&pScore->level[item+1],	//move into [10] ..
				&pScore->level[item],	//from [9], etc etc
				sizeof(tHiScoreEntry));
	//copy "curr" inits, score and words to correct hi score table
	memcpy(&pScore->level[level], &curr, sizeof(tHiScoreEntry));
}

std::string Score::inits(int mode, int diff, int level)
{
	return getLevel(mode, diff)->level[level].inits;
}

unsigned int Score::words(int mode, int diff, int level)
{
	return getLevel(mode, diff)->level[level].words;
}

unsigned int Score::score(int mode, int diff, int level)
{
	return getLevel(mode, diff)->level[level].score;
}

unsigned int Score::fastest(int mode, int diff, int level)
{
	return getLevel(mode, diff)->level[level].fastest;
}

void Score::setCurr(tHiScoreEntry &curr)
{
	_curr = curr;
}

//return a pointer to the correct array depending on mode and difficulty level
tHiScoreLevels *Score::getLevel(int mode, int diff)
{
	if (diff >= DIF_EASY && diff < DIF_MAX) --diff; else diff = 0;
	switch ((eGameMode)mode)
	{
	case GM_SPEEDER: 	return &_hiScoreS6[diff];
	case GM_TIMETRIAL: 	return &_hiScoreTT[diff];
	case GM_ARCADE:     return &_hiScoreRA[diff];

	case GM_REWORD:
	default:
	 	return &_hiScore[diff];
	}
}


//////////////////////////////////////////////////////////////////////////////////
//	QuickState save class member functions
//	Used for save/resore mid game...
//////////////////////////////////////////////////////////////////////////////////

QuickState::QuickState()
{
#ifdef PANDORA
	//pandora .pnd directory structure requires a unnamed directory for data written back to the SD card,
	//so create a score directory in the startup/run .sh script and read/write score files in there.
	_quickstatefile = "./.quickstate.save";
#else
	_quickstatefile = RES_BASE + ".quickstate.save";
#endif
}

bool QuickState::quickStateSave()
{
	//quick save state for restore:
	//score, words, dictionary shuffle seed, game type, game diff, dict file,
 	std::ofstream outfile(_quickstatefile.c_str(), std::ios::out|std::ifstream::trunc);
	if (outfile.is_open())
	{
		outfile << "wordfile=" << _qStateSave._wordfile << std::endl;
		outfile << "words=" << _qStateSave._words << std::endl;
		outfile << "score=" << _qStateSave._score << std::endl;
		outfile << "diff=" << _qStateSave._diff << std::endl;
		outfile << "mode=" << _qStateSave._mode << std::endl;
		outfile << "seed=" << _qStateSave._seed << std::endl;
	}
	return true;
}

bool QuickState::quickStateLoad()
{
	std::string line, key, value;
	std::ifstream infile (_quickstatefile.c_str(), std::ios_base::in);
	if (infile.is_open())
	{
		while (std::getline(infile, line, '\n'))
		{
			pptxt::splitKeyValuePair(line, key, value);
			if (key == "wordfile")
				_qStateSave._wordfile = value;
			else if (key == "words")
				_qStateSave._words = atoi(value.c_str());
			else if (key == "score")
				_qStateSave._score = atoi(value.c_str());
			else if (key == "diff")
				_qStateSave._diff = atoi(value.c_str());
			else if (key == "mode")
				_qStateSave._mode = atoi(value.c_str());
			else if (key == "seed")
				_qStateSave._seed = atoi(value.c_str());
		}
		return true;
	}
	return false;
}

//return true if .quickState.save file exists
bool QuickState::quickStateExists()
{
	return std::ifstream(_quickstatefile.c_str());
}

//delete the quick state save file -once reload has been selected, so user
//must save state again to be able to resume later. I.e user shouldn't be
//able to keep resuming the same game and get better scores each time.
void QuickState::quickStateDelete()
{
	std::remove(_quickstatefile.c_str());
}

void QuickState::setQuickState(const tQuickStateSave &qss)
{
	_qStateSave = qss;
}
void QuickState::getQuickState(tQuickStateSave &qss)
{
	qss = _qStateSave;
}




