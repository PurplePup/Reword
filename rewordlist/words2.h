//words2.h

#ifndef _WORDS2_H
#define _WORDS2_H

#include <deque>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "stdio.h"

#include "../reword/words.h"
#include "./tinyxml/tinyxml.h"


class Words2 : public Words
{
public:
	Words2();
	Words2(const std::string &wordFile);
	~Words2();

	//Add words from the xdxf dictionary to the internal lists
	bool xdxfBuildDict(const std::string &dictFile, bool bUpdateDef);

	//build the output dictionary from all loaded words
	bool filterGameWords();   //const std::string &dictFile = "", bool bUpdateDef = false);

	bool save(std::string outFile);
	bool empty();

	Words2 & operator += (const Words2 &w2)
	{
		// Check for self-assignment
		if (this != &w2)      // not same object, so add all of 'w2' to 'this'
		{
			this->Words::operator+=(w2);	//call Words += operator overload first
			for (int i = TARGET_MAX; i >= SHORTW_MIN; --i)
			{
				_wordSet[i].insert(w2._wordSet[i].begin(), w2._wordSet[i].end());
			}
		}
		return *this;
	}
	const Words2 operator+(const Words2 &other) const
	{
		return Words2(*this) += other;	//call += operator overload (as it's already there)
	}

	Words2 & operator -= (const Words2 &w2)
	{
		// Check for self-assignment
		if (this != &w2)      // not same object, so del all of 'w2' from 'this'
		{
			this->Words::operator-=(w2);	//call Words overload first
			for (int i = TARGET_MAX; i >= SHORTW_MIN; --i)
			{
//				_wordSet[i].erase(w2._wordSet[i].begin(), w2._wordSet[i].end());
				tWordSet temp;
				std::set_difference( _wordSet[i].begin(), _wordSet[i].end(),
									 w2._wordSet[i].begin(),w2._wordSet[i].end(),
									 std::inserter(temp, temp.begin()));
				temp.swap(_wordSet[i]);
			}

		}
		return *this;
	}
	const Words2 operator-(const Words2 &other) const
	{
		return Words2(*this) -= other;	//call -= operator overload (as it's already there)
	}

//	int wordSetCount() { int c(0); for(int i=SHORTW_MIN; i<=TARGET_MAX; c+=_wordSet[i++].size()); return c; }
//	int mapAllCount() { return _mapAll.size(); }

protected:

	bool 			xdxfOpenDict(const std::string &dictFile);	//open generic xml file
	void 			xdxfCloseDict();
	TiXmlElement* 	xdxfFirstWord();
	TiXmlElement* 	xdxfNextWord(TiXmlElement* ar, std::string &word, std::string &def);

	void addWordsToSets();	//add to valid sets (one set per word length)
	int saveWordMap(FILE *& fp, tWordMap &wmOrig, tWordSet &wsFilt);

	TiXmlDocument * _doc;
	int		_countXdxfWords;
	int		_countXdxfSkipped;
	int		_countXdxfMatched;
	int		_countXdxfMissing;

	tWordSet _wordSet[TARGET_MAX+1];	//use 1..n for actual word length (as index) during reword.txt build
};

#endif //_WORDS2_H
