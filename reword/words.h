//words.h

#ifndef _WORDS_H
#define _WORDS_H

#include <deque>
#include <set>
#include <map>
#include <vector>
#include <string>

#include <ios>
#include <iostream>

#include "stdio.h"

#include "states.h"
#include "random.h"

//SDL header for ticks - now NOT included here so we can use with or without SDL libraries
//include SDL.h in your own code before including words.h if you want to use SDL_GetTicks()
//#include "SDL.h"

//define the shortest word we handle
#define SHORTW_MIN	((int)3)
//define the 'big' target words to be used as the game words. Was just 6 letter words,
//but now we can expand it to pick words between 6 and ...
#define TARGET_MIN	((int)6)
#define TARGET_MAX	((int)8)


struct DictWord
{
	std::string _word;
	int			_level;			//1=easy, 2=med, 3=hard (0=undefined/easy)
	std::string _description;
	bool		_personal;		//a personally entered word (not in dict)
	bool 		_found;

	void clear()
	{
		_word = "";
		_level = 0;
		_description = "";
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
			this->_personal = dw._personal;
			this->_found = dw._found;
		}
		return *this;
	};

//	bool operator < (const DictWord &rhs)	//for sorting
//	{
//		return this->_word < rhs._word;
//	}

	bool operator () (const DictWord &dw1, const DictWord &dw2) const
	{
		return dw1._word < dw2._word;
	}

	bool operator () (const DictWord &dw1, const std::string &word) const
	{
		return dw1._word < word;
	}
};

typedef std::map<std::string, DictWord> tWordMap;	//map of random numbers and the (same length) words available
typedef std::map<std::string, bool> tWordsInTarget;	//map of words during a level and if its been found by the player
typedef std::set<std::string> tWordSet;				//for unique set of words
typedef std::vector<std::string> tWordVect;

/*
class RandInt
{
public:
	RandInt() { 
#ifdef _SDL_H
#ifdef WIN32
#pragma message("words.h: Using SDL_GetTicks() to seed random\n")
#endif
		setSeed(SDL_GetTicks()); 
#else
#ifdef WIN32
#pragma message("words.h: Using ctime to seed random\n")
#endif
		m_rnd.Randomize();	//uses ctime to seed
#endif
	}	//incase setSeed not called
	void setSeed(unsigned int seed) {
		m_rnd.SetRandomSeed(seed);
		m_rnd.Randomize();
	}
	int operator() (int limit)
	{
		return (int)m_rnd.Random(limit);
	}
	CRandom	m_rnd;					//random number generator class
};
*/

class Words
{
public:
	Words();
	Words(const std::string &wordFile);
	void setList(bool bOn = true) { _bList = bOn; }
	void setDebug(bool bOn = true) { _bDebug = bOn; }

	bool rejectWord(const std::string &strWord);		//true if word loaded not useable
	bool load(std::string wordFile = std::string(""), 	//load a wordlist and exclude
				unsigned int rndSeed = 0,				//duplicates, too many etc
				unsigned int startAtWord = 0);

	unsigned int wordsLoaded() { return _total; };		//before exclusions, duff words etc
	unsigned int size() { return (unsigned int)_mapAll.size(); }	//current size
	
	bool nextWord(std::string &retln, eGameDiff level, eGameMode mode, bool reloadAtEnd=true);
	const tWordsInTarget getWordsInTarget() { return _wordsInTarget; };
	int wordsOfLength(unsigned int i) { if (i > TARGET_MAX) return 0; else return _nWords[i]; };
	int checkWordsInTarget(std::string &testWord);
	DictWord getDictForWord(std::string &wrd);
//	unsigned int wordsInLevel(unsigned int i) { if (i >= DIF_MAX) return 0; else return _nInLevel[i]; };

	std::string getWord6() { return _word._word; };			//curr word
	int			getWordLevel() { return _word._level; };		//curr word level
	std::string getWordDesc() { return _word._description; };	//curr word description

	Words & operator+=(const Words &w)
	{
	    // Check for self-assignment
	    if (this != &w)      // not same object, so add all of 'w' to 'this'
	    {
	    	//we must iterate through and assign, as there is a map and a vect to modify
	    	//We can't just use map.insert(begin,end) as we need to check for existence and
	    	//insert into vect if not there too.
			tWordMap::const_iterator pos;
			for (pos = w._mapAll.begin(); pos != w._mapAll.end(); ++pos)
			{
				//only add if not already exists - this save us from searching the m_vect6 vector each time
				if (this->_mapAll.find(pos->first) == this->_mapAll.end())
				{
					this->_mapAll[pos->first] = pos->second;
					if (6 == pos->first.length())		//is a 6 letter word
						this->_vecTarget.push_back(pos->first);	//so also add to valid 6 letter word vector
				}
			}
		}
		return *this;
	}
	const Words operator+(const Words &other) const
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
				//int n(0);
				tWordVect::iterator vpos;
				for (vpos = this->_vecTarget.begin(); vpos != this->_vecTarget.end(); ++vpos)
				{
					if (*vpos == pos->first)
					{
						//std::cout << pos->first << " erased after " << n+1 << " reads" << std::endl;
						this->_vecTarget.erase(vpos);
						break;
					}
					//n++;
				}
			}
		}
		return *this;
	}
	const Words operator-(const Words &other) const
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
	
	tWordMap 		_mapAll;				//all words - for
	tWordVect		_vecTarget;				//vector to hold all 6 letter words in a rnd order
	tWordVect::const_iterator _vecTarget_it;//m_vect6 iterator
	DictWord		_word;					//current 6 letter word to find etc
	tWordsInTarget 	_wordsInTarget;			//map of sub words (ie 3,4,5,6 letter for word6) with a "found" flag to say player got it
	int 			_nWords[TARGET_MAX+1];	//count of number of words of each length to be found in 3, 4, 5 & 6 letter word lists
//	unsigned int	_nInLevel[DIF_MAX];		//number of words in each level (to disable a level if == 0)

	int				_total;					//total words loaded
	int				_ignored;				//number of words ignored
	bool 			_bList;					//output processing msgs to console
	bool			_bDebug;				//output detail 'debug' to console?

	std::string 	_wordFile;				//saved when load() called to allow nextWord() to reload
};


#endif //_WORDS_H
