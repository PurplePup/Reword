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

class CWords
{
public:
	CWords();
	CWords(const std::string &wordFile);
	void setList(bool bOn = true) { m_bList = bOn; }
	void setDebug(bool bOn = true) { m_bDebug = bOn; }

	bool load(std::string wordFile = std::string(""), 	//load a wordlist and exclude 
				unsigned int rndSeed = 0);				//duplicates, too many etc

	unsigned int wordsLoaded() { return m_total; };		//before exclusions, duff words etc
	unsigned int size() { return (unsigned int)m_mapAll.size(); }	//current size
	
	bool nextWord(std::string &retln, eDifficulty level, eGameMode mode, bool reloadAtEnd=true);
	const tWordsInTarget getWordsInTarget() { return m_wordsInTarget; }; 
	int wordsOfLength(unsigned int i) { if (i > 3) return 0; else return m_nWords[i]; };
	int checkWordsInTarget(std::string &testWord);
	DictWord getDictForWord(std::string &wrd);
	unsigned int wordsInLevel(unsigned int i) { if (i >= DIF_MAX) return 0; else return m_nInLevel[i]; };

	std::string getWord6() { return m_word._word; };			//curr word
	int			getWordLevel() { return m_word._level; };		//curr word level
	std::string getWordDesc() { return m_word._description; };	//curr word description

	CWords & operator+=(const CWords &w)
	{
	    // Check for self-assignment
	    if (this != &w)      // not same object, so add all of 'w' to 'this'
	    {
	    	//we must iterate through and assign, as there is a map and a vect to modify
	    	//We can't just use map.insert(begin,end) as we need to check for existence and
	    	//insert into vect if not there too.
			tWordMap::const_iterator pos;
			for (pos = w.m_mapAll.begin(); pos != w.m_mapAll.end(); ++pos)
			{
				//only add if not already exists - this save us from searching the m_vect6 vector each time
				if (this->m_mapAll.find(pos->first) == this->m_mapAll.end())
				{
					this->m_mapAll[pos->first] = pos->second;
					if (6 == pos->first.length())		//is a 6 letter word
						this->m_vect6.push_back(pos->first);	//so also add to valid 6 letter word vector 
				}
			}
		}
		return *this;
	}
	const CWords operator+(const CWords &other) const 
	{
		return CWords(*this) += other;	//call += operator overload (as it's already there)
	}

	CWords & operator-=(const CWords &w)
	{
	    // Check for self-assignment
	    if (this != &w)      // not same object, so remove all of 'w' from 'this'
		{
			//iterate through and remove from vect too, time consuming but necessary
			tWordMap::const_iterator pos;
			for (pos = w.m_mapAll.begin(); pos != w.m_mapAll.end(); ++pos)
			{
				this->m_mapAll.erase(pos->first);	//erase by value

				//find the same word in the vect6 and erase it
				//int n(0);
				tWordVect::iterator vpos;
				for (vpos = this->m_vect6.begin(); vpos != this->m_vect6.end(); ++vpos)
				{
					if (*vpos == pos->first)
					{
						//std::cout << pos->first << " erased after " << n+1 << " reads" << std::endl;
						this->m_vect6.erase(vpos);
						break;
					}
					//n++;
				}
			}
		}
		return *this;
	}
	const CWords operator-(const CWords &other) const 
	{
		return CWords(*this) -= other;	//call -= operator overload (as it's already there)
	}

protected:

	void reset();
	void clearCurrentWord();
	bool checkCurrentWord6(tWordVect::const_iterator &it);
	bool wordInWord(const char * shortWord, const char * word6);
	int findWordsInWord6(tWordMap &shortwords, const char *word6);
	bool splitDictLine(std::string line, DictWord &dict);
	
	tWordMap 		m_mapAll;				//all words - for 
	tWordVect		m_vect6;				//vector to hold all 6 letter words in a rnd order
	tWordVect::const_iterator m_vect6_it;	//m_vect6 iterator
	DictWord		m_word;					//current 6 letter word to find etc
	tWordsInTarget 	m_wordsInTarget;		//map of sub words (ie 3,4,5,6 letter for word6) with a "found" flag to say player got it
	int 			m_nWords[4];			//count of number of words of each length to be found in 3, 4, 5 & 6 letter word lists
	unsigned int	m_nInLevel[DIF_MAX];	//number of words in each level (to disable a level if == 0)

	int				m_total;				//total words loaded
	int				m_ignored;				//number of words ignored
	bool 			m_bList;				//output processing msgs to console
	bool			m_bDebug;				//output detail 'debug' to console?

	std::string 	m_wordFile;				//saved when load() called to allow nextWord() to reload
};


#endif //_WORDS_H
