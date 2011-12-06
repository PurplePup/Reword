////////////////////////////////////////////////////////////////////
/*

File:			words.cpp

Class impl:		Words

Description:	Class to handle most of the word manipulation in the REWORD game

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3		25.02.2007	Rework word matching algorithms to work on-the-fly so we don't
										need to pre-create a formatted wordlist with all words matching
										each 6 letter word.  This also helps prevent people looking up
										the matching words during play.
										Rewordlist utility app is still used to initially filter wordlists,
										but now mainly used to add dictionary definitions.
									Header file excludes SDL_GetTicks call when seeding random numbers if
										called from app (rewordlist) that doesn't use SDL
				0.4		09.11.2007	Modify load() to pass in dict container so we can load diff files into
										diff containers. Mainly used for exclusion/inclusion file lists
									Added operator overloading
				0.5.2	23.09.2010	Added ability to load longer words

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

#include "words.h"
#include "helpers.h"
#include "platform.h"

#include <fstream>
#include <ios>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <iterator>

#include <string.h>	//for strchr etc



//using namespace std;


Words::Words() :
	_bList(false), _bDebug(false), _wordFile("")
{
	reset();
}

Words::Words(const std::string &wordFile) :
	_bList(false), _bDebug(false), _wordFile(wordFile)
{
	load(wordFile);	//calls reset() etc
}

void Words::clearCurrentWord()
{
	_word.clear();
	_wordsInTarget.clear();
	for (int i=0; i<=TARGET_MAX; _nWords[i++]=0);
}

void Words::reset()
{
	//reset all dictionary vars used
	_mapAll.clear();

    //and counter stats
	_stats.clear();

	//and any current word values, counts etc
	clearCurrentWord();
}

//split up the line read in from the word list file
//up to 3 params per line:
//	WORD|level|description
//only the word is mandatory, the other two default to level=0 and description=""
bool Words::splitDictLine(std::string text, DictWord &dictword)
{
	std::string::size_type end;
	int count = 0;
	std::string newword;

	dictword.clear();
    do
    {
        end = text.find('|');	//pipe chr for "WORD|level|Description"
        if (end == std::string::npos)
            end = text.length() + 1;
		newword = text.substr(0,end);
		pp_s::trim(newword, " \r\n\t\'");
		switch (count)
		{
		case 0:	pp_s::makeUpper(newword);
				dictword._word = newword;
				break;
		case 1:	dictword._level = atoi(newword.c_str());
				break;
		case 2:	dictword._description = newword;
				break;
		default:break;
		}
		count++;
		text.replace(0,end+1,"");	//remove the string from the line
    } while (text.length());

	return dictword._description.length() > 0;
}

//return true if word passed in is not useable in the game due to size or content
bool Words::rejectWord(const std::string &strWord)
{
	const size_t found = strWord.find_first_of("\' .-;,");
	if (found!=std::string::npos)  //ignore words with apostrophies or whitespace
	{
		if (_bDebug) std::cout << "DEBUG: Ignore (\') " << strWord.c_str() << std::endl;
		return true;
	}

	const int wordLen = strWord.length();
	if ((wordLen < SHORTW_MIN) || (wordLen > TARGET_MAX))  //ignore out-size words
	{
		if (_bDebug) std::cout << "DEBUG: Ignore (size) " << strWord.c_str() << std::endl;
		return true;	//rejected
	}
	return false;
}


//randomizer fn
//ptrdiff_t wrandom (ptrdiff_t i) { return rand()%i;}

//using restartable rnd function to re-generate same
//random sequence if given same seed again (for resume games)
unsigned int g_rndSeed  = 1234;

#ifdef WIN32
ptrdiff_t wrandom (ptrdiff_t i) { return rand_s(&g_rndSeed)%i;}
#else
ptrdiff_t wrandom (ptrdiff_t i) { return rand_r(&g_rndSeed)%i;}
#endif
// pointer object to it:
ptrdiff_t (*pwrandom)(ptrdiff_t) = wrandom;


//call with default param (blank wordFile string) to reload previous file
//Load all words from wordlist file. The nextWord() function locates next
//valid word and all its shortedr words.
bool Words::load(const std::string &wordFile, unsigned int rndSeed, unsigned int startAtWord)
{
	if (!wordFile.length() && !_wordFile.length())
	{
		std::cerr << "Wordfile not specified, cannot load" << std::endl;
		return false;	//no previously opened/saved file
	}

	//reset all containers and working vars and totals to 0 etc
	reset();

	tWordSet dictSet;	//to remove duplicates... discarded after load()
	std::pair<tWordSet::const_iterator, bool> dictPair;

	if (wordFile.length()) _wordFile = wordFile;	//save it for any reload

	std::string lnwrd;
	DictWord dictWord;

	std::ifstream ifs1 (_wordFile.c_str(), std::ifstream::in);	//open the file
	if (ifs1.is_open() && !ifs1.eof())
	{
		if (_bDebug) std::cout << _wordFile << std::endl;

		int wordLen = 0;
		while (std::getline(ifs1, lnwrd))
		{
			_stats._total++;

			splitDictLine(lnwrd, dictWord);
			lnwrd = dictWord._word;
			wordLen = lnwrd.length();

			if (rejectWord(lnwrd))
			{
				_stats._ignored++;
				continue;
			}

			if (_bDebug) std::cout << "Line " << _stats._total << ": " << lnwrd.c_str() << std::endl;

			dictPair = dictSet.insert(lnwrd);
			if (!dictPair.second)	//not inserted so is a duplicate word
			{
				if (_bDebug) std::cout << "Line " << _stats._total << " Duplicate: " << lnwrd.c_str() << std::endl;
				_stats._ignored++;
				continue;
			}

			//check if level given is valid and increment count
			if (dictWord._level < (int)DIF_EASY || dictWord._level > (int)DIF_MAX)
				dictWord._level = 0;

			//add word to dict
			if (_mapAll.find(lnwrd) == _mapAll.end())			//not found
			{
				_mapAll[lnwrd] = dictWord;		//so insert it in the ALL word map
				if (wordLen >= TARGET_MIN && wordLen <= TARGET_MAX)
					_vecTarget.push_back(lnwrd);	//so also add to valid 6,7,.. letter word vector used for nextWord()
			}
		}
		ifs1.close();

		if (_bDebug) std::cout << "Ignored: " << _stats._ignored << std::endl;

//#ifndef _DEBUG	//curr only shuffle in release version so we can test things in debug
		//randomize the 6 letter vector to be the order we get each new word in
//		if (rndSeed) g_randInt.setSeed(rndSeed);
//		std::random_shuffle(_vect6.begin(), _vect6.end(), g_randInt);

		// random generator function:
		if (rndSeed) g_rndSeed = rndSeed; //srand(rndSeed);
		std::random_shuffle(_vecTarget.begin(), _vecTarget.end(), pwrandom);

		//output the start of the sorted list to check order
/*		std::cout << "DEBUG: rnd seed = " << rndSeed << std::endl;
		std::cout << "List first 20..." << std::endl;
		std::copy(_vecTarget.begin(), _vecTarget.begin()+20, std::ostream_iterator<std::string>(std::cout, "\n"));	//list first 20*/
//#endif
		//set the 6word iterator to start (or a specific position)
		if (startAtWord >=  (unsigned int)_vecTarget.size())
			startAtWord = 0;	//invalid for size of word llist so just reset to 0
		_vecTarget_it = _vecTarget.begin() + startAtWord;

		return true;
	}

	std::cerr << "Failed to load word file " << wordFile << std::endl;
	return false;
}

//determine if the letters in wordShort are in wordTarget
//i.e. do all the chars in short word xyz exist in long word xaybzc
//ShortWord can be made up from some or all letters in longWord (without using letters twice)
bool Words::wordInWord(const char * wordShort, const char * wordTarget)
{
	int ilongLen = strlen(wordTarget);
	int ishortLen = strlen(wordShort);

	if (!ilongLen || !ishortLen) return false;	//invalid word

	int il, is;		//longword and shortword counters
	int mask = 0;	//used to check if letters at specific position already tested for (int is ok as words are only ever 3..8 char long)
	int found = 0;	//number of letters found
	for (is = 0; is < ishortLen; is++)
	{
		for (il = 0; il < ilongLen; il++)
		{
			if (wordTarget[il] == wordShort[is])	//char match
			{
				if ((mask & (1<<il)) == 0)	//if not already matched
				{
					mask |= 1<<il;	//set mask and go for next
					++found;
					break;
				}
				//else try next available letter
			}
		}
		if (found != is+1) break; //last loop failed to find letter
	}
	return found == ishortLen;
}

//return all the (short word) shortwords that are in (longer word) wordTarget
//Also updates _nWords[] with count of words of each length, used in checkCurrentwordTarget()
int Words::findWordsInWordTarget(tWordMap &shortwords, const char *wordTarget)
{
	int count = 0;
	if (_bDebug) std::cout << wordTarget << ": ";
	for (tWordMap::const_iterator shtwrd = shortwords.begin (); shtwrd != shortwords.end (); ++shtwrd)
	{
//		if (strcmp((*shtwrd).second.c_str(), wordTarget) == 0) continue;	//exclude same word?

		if (wordInWord( (*shtwrd).first.c_str(), wordTarget ))
		{
			if (_bDebug) std::cout << (*shtwrd).first.c_str() << ", ";

			_wordsInTarget.insert(tWordsInTarget::value_type( (*shtwrd).first.c_str(), false ));	//false = each word not "found" yet
			_nWords[(*shtwrd).first.length()]++;		//ignore 0,1,2 and start at 3 as min word len is 3
			count++;
		}
	}
	if (_bDebug) std::cout << std::endl;
	return count;
}


bool Words::checkCurrentWordTarget(const std::string &wordTarget)
{
	bool bOk = true;

	//fill in counters...
	findWordsInWordTarget(_mapAll, wordTarget.c_str()); //side effect - fills _nWords[]
	const int wordTargetLength = (int)wordTarget.length();
	const int wordTargetStart = wordTargetLength - 3;	//for max out checking

	int iShortWords = 0, i = 0;
	//if we ever move to displaying matched words in a wrapping list we could get more on screen
	//so would need to test for an overall too high number rather than per column.
	//Also, as I've just changed it to a higher target word letter count, only check if too many letters
	//from 4 back as only 4 columns shown on screen (so for a 6 letter word check 3, 4, 5 and 6 - but
	//for a 7 letter word check 4, 5, 6 and 7
	for (i = wordTargetStart; i <= wordTargetLength; ++i)
	{
		iShortWords += _nWords[i];
		//check if any exceed our rowcount max for a particular column
//		if (_nWords[i] > MAX_WORD_COL)
//		{
//			if (_bDebug) std::cout << "Too many size " << i << " (" << _nWords[i] << ") for word : " << wordTarget << std::endl;
//			bOk = false;
//		}
	}

	//only need to exclude target word if there are no short words at all for it
	if (bOk && iShortWords == 0)	//exclude target word due to missing (short) 3, 4, 5 letter words
	{
		if (_bDebug) std::cout << "All short words missing for word : " << wordTarget << std::endl;
		bOk = false;
	}

	return bOk;
}

//get the next 6to8 letter word to use in the game
bool Words::nextWord(std::string &retln, eGameDiff level,  eGameMode mode, bool reloadAtEnd /*=true*/)
{
	bool bOk = true;
	int failsafe = _vecTarget.size(); //size of the whole vector, so allow to loop once to find next
	tWordMap::iterator mapit;
	do
	{
		//clear internal variables holding current word
		clearCurrentWord();

		if (_vecTarget_it == _vecTarget.end())
		{
			//reload the dictionary
			retln = "";
			if (!reloadAtEnd || !load()) return false; //reached list end or failed reload
		}

		mapit = _mapAll.find(*_vecTarget_it);		//find word from vect in the wordTarget dict map
		if (mapit != _mapAll.end())					//found, so check the level
		{
			if ( (bOk = ((*mapit).second._level <= (int)level)) )//make sure word is at or below current difficulty level
			{
				if ( (bOk = checkCurrentWordTarget(*_vecTarget_it)) )
				{
					//set the "current word" to that just found
					_word = (*mapit).second;

					if (mode > GM_REWORD)
					{
						//quick and dirty to strip non target length words from those just found
						//as speeder and time trial modes only use the higher target words
						for (int i=0; i<=TARGET_MAX; ++i)
							if (i != (int)_word._word.length()) _nWords[i] = 0;	//keep selected word count
					}
				}
			}
		}
		++_vecTarget_it;	//next word
	} while (--failsafe && !bOk);

	if (!bOk) _word._word = "XXXXXX";	//err in word list - too many or missing

	retln = _word._word;

	return bOk;
}


//test to see if testWord is in the target list and if so set it to 'found'
//otherwise return a 0 to indicate already found, or -1 for not found.
int Words::checkWordsInTarget(std::string &testWord)
{
	//it could still be a 3, 4, 5, or another 6 letter word!
	tWordsInTarget::iterator it = _wordsInTarget.find(testWord);
	if (it != _wordsInTarget.end())	//found
	{
		if ((*it).second == true) return 0;	//already found
		(*it).second = true;	//indicate its now been found
	}
	else return -1;	//not found

	return 1;	//found and set to 'found' in list

}

//return a full dictionary (word, level and description) for the given word
DictWord Words::getDictForWord(std::string &wrd)
{
	tWordMap::iterator it = _mapAll.find(wrd);
	if (it == _mapAll.end())
	{
		return DictWord();	//not found - blank struct
	}
	return (*it).second;

}


