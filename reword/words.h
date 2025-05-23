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
	int			_level = 0;			// 1=easy, 2=med, 3=hard (0=undefined/easy)
	std::string _description;
	std::vector<std::string> _prematch;	// a vector of strings found
	int			_index = 0;			// final index position of word in rw2 file prematch format 

	bool		_personal = false;	// a personally entered word (not in dict)
	bool 		_found = false;		// in-play flag to indicate found/entered by player

    DictWord() {
		clear(); 
	}

	void clear()
	{
		_word.clear();
		_level = 0;
		_description.clear();
		_prematch.clear();
		_index = 0;
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
			this->_index = dw._index;
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
    void clear() 
	{ 
		_total = _ignored = 0;
		_countLevels.resize(3);
		_countScore.resize(100);
    }
	int		_total = 0;					//total words loaded
	int		_ignored = 0;				//number of words ignored

	std::vector<int> _countLevels;      //number or easy words, med words and hard words
	std::vector<int> _countScore;       //number of words for each score (to help define thresholds)
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
	virtual bool rejectDefinition(const DictWord& dictWord) { return false; }

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

	Words & operator+=(const Words &w);
	Words operator+(const Words &other) const;

	Words & operator-=(const Words &w);
	Words operator-(const Words &other) const;

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

	Stats           _stats;					// stats to display by rewordlist on completion
};


#endif //_WORDS_H
