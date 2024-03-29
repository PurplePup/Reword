//words.h

#if !defined _WORDS_H
#define _WORDS_H

#include <deque>
#include <set>
#include <map>
#include <vector>
#include <string>

#include <ios>
#include <iostream>

#include <stdio.h>
#include <memory>

#include "platform.h"
#include "states.h"

//SDL header for ticks - now NOT included here so we can use with or without SDL libraries
//include SDL.h in your own code before including words.h if you want to use SDL_GetTicks()
//#include "SDL.h"

//define the shortest word we handle
#define SHORTW_MIN	((int)3)
//define the 'big' target words to be used as the game words. Was just 6 letter words,
//but now we can expand it to pick words between 6 and ...
#define TARGET_MIN	((int)6)
#define TARGET_MAX	((int)8)

#define MAX_REWORD_DESCRIPTION    1000

struct DictWord
{
	std::string _word;
	int			_level;			//1=easy, 2=med, 3=hard (0=undefined/easy)
	std::string _description;
	std::vector<std::string> _prematch;	// a vector of strings found

	bool		_personal;		//a personally entered word (not in dict)
	bool 		_found;

    DictWord() {
		clear(); 
	}

	void clear()
	{
		_word.clear();
		_level = 0;
		_description.clear();
		_prematch.clear();
		_personal = false;
		_found = false;
	};

	DictWord& operator=(const DictWord &dw) {
	    // Check for self-assignment
	    if (this != &dw)      // not same object
		{
			this->_word = dw._word;
			this->_level = dw._level;
			this->_description = dw._description;
			this->_prematch = dw._prematch;
			this->_personal = dw._personal;
			this->_found = dw._found;
		}
		return *this;
	};

	bool operator () (const DictWord &dw1, const DictWord &dw2) const
	{
		return dw1._word < dw2._word;
	}

	bool operator () (const DictWord &dw1, const std::string &word) const
	{
		return dw1._word < word;
	}
};

using tWordMap = std::map<std::string, DictWord>;// , std::less< >> ;		//map of random numbers and the (same length) words available
using tWordsInTarget = std::map<std::string, bool>;// , std::less< >> ;	//map of words during a level and if it's been found by the player
using tWordSet = std::set<std::string>;// , std::less< >> ;				//for unique set of words
using tWordVect = std::vector<std::string>;


struct Stats
{
    Stats() { clear(); }
    void clear() { _total = _ignored = 0;
                    memset(_countLevels, 0, sizeof(_countLevels));
                    memset(_countScore, 0, sizeof(_countScore));
                }
	int		_total;					//total words loaded
	int		_ignored;				//number of words ignored

	int     _countLevels[3];                //number or easy words, med words and hard words
	int     _countScore[100];               //number of words for each score (to help define thresholds)
};


class Words
{
public:
	Words();
	virtual ~Words() = default;
	explicit Words(const std::string &wordFile);
	void setList(bool bOn = true) { _bList = bOn; }
	void setDebug(bool bOn = true) { _bDebug = bOn; }

	bool rejectWord(const std::string &strWord);		//true if word loaded not useable
	virtual bool load(const std::string &wordFile = "", 		//load a wordlist and exclude
				unsigned int rndSeed = 0,				//duplicates, too many etc
				unsigned int startAtWord = 0);
	unsigned int wordsLoaded() const { return _stats._total; };		//before exclusions, duff words etc
	std::size_t size() const { return _mapAll.size(); }	//current size

	bool nextWord(std::string &retln, eGameDiff level, eGameMode mode, bool reloadAtEnd=true);
	tWordsInTarget getWordsInTarget() const { return _wordsInTarget; };
	int wordsOfLength(unsigned int i) const { if (i > TARGET_MAX) return 0; else return _nWords[i]; };
	int checkWordsInTarget(std::string &testWord);
	DictWord getDictForWord(std::string &wrd);

	std::string getWordTarget() const { return _word._word; };		//curr word target
	int			getWordLevel() const { return _word._level; };		//curr word level
	std::string getWordDesc() const { return _word._description; };	//curr word description

	Words & operator+=(const Words &w)
	{
	    // Check for self-assignment
	    if (this != &w)      // not same object, so add all of 'w' to 'this'
	    {
	    	//we must iterate through and assign, as there is a map and a vect to modify
	    	//We can't just use map::insert(begin,end) as we need to check for existence and
	    	//insert into vect if not there too.
			tWordMap::const_iterator pos;
            tWordMap::iterator foundpos;
			for (pos = w._mapAll.begin(); pos != w._mapAll.end(); ++pos)
			{
                foundpos = this->_mapAll.find(pos->first);
				//only add if not already exists - this saves us from searching the _vecTarget vector each time
				if (foundpos == this->_mapAll.end())
				{
				    //check we don't overwrite a valid value with a blank, but anything valid overwrites
					this->_mapAll[pos->first]._word = pos->second._word;
					if (!pos->second._description.empty())
                        this->_mapAll[pos->first]._description = pos->second._description;
                    if (pos->second._level > 0)
                        this->_mapAll[pos->first]._level = pos->second._level;

					if ((pos->first.length() >= TARGET_MIN) && (pos->first.length() <= TARGET_MAX))  //is a 6to8 target word
						this->_vecTarget.push_back(pos->first);	//so also add to valid 6to8 letter word vector
				}
			}
		}
		return *this;
	}
	Words operator+(const Words &other) const
	{
		return Words(*this) += other;	//call += operator overload (as it's already there)
	}

	Words & operator-=(const Words &w)
	{
	    // Check for self-assignment
	    if (this != &w)      // not same object, so remove all of 'w' from 'this'
		{
			//iterate through and remove from vect too, time consuming but necessary
			tWordMap::const_iterator pos;
			for (pos = w._mapAll.begin(); pos != w._mapAll.end(); ++pos)
			{
				this->_mapAll.erase(pos->first);	//erase by value

				//find the same word in the vect6 and erase it
				tWordVect::iterator vpos;
				for (vpos = this->_vecTarget.begin(); vpos != this->_vecTarget.end(); ++vpos)
				{
					if (*vpos == pos->first)
					{
						//std::cout << pos->first << " erased after " << n+1 << " reads" << std::endl;
						this->_vecTarget.erase(vpos);
						break;
					}
				}
			}
		}
		return *this;
	}
	Words operator-(const Words &other) const
	{
		return Words(*this) -= other;	//call -= operator overload (as it's already there)
	}

protected:

	void reset();
	void clearCurrentWord();
	bool checkCurrentWordTarget(const std::string &wordTarget);
	bool wordInWord(const char* wordShort, const char* wordTarget);
	int findWordsInWordTarget(tWordMap &shortwords, const char *word6);
	bool splitDictLine(std::string line, DictWord &dict);

	tWordMap 		_mapAll;				//all words - for full wordlist to test against (during game)
	tWordVect		_vecTarget;				//vector to hold all 6,7,8 letter words in a rnd order (during game)
	tWordVect::const_iterator _vecTarget_it;//working vect target iterator
	DictWord		_word;					//current 6 letter word to find etc
	tWordsInTarget 	_wordsInTarget;			//map of sub words (ie 3,4,5,6 letter for word6) with a "found" flag to say player got it
	int 			_nWords[TARGET_MAX+1];	//count of number of words of each length to be found in 3, 4, 5 & 6 letter word lists
//	unsigned int	_nInLevel[DIF_MAX];		//number of words in each level (to disable a level if == 0)

	bool 			_bList;					//output processing msgs to console
	bool			_bDebug;				//output detail 'debug' to console?

	std::string 	_wordFile;				//saved when load() called to allow nextWord() to reload

	Stats           _stats;
};


#endif //_WORDS_H
