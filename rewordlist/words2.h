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
	explicit Words2(const std::string &wordFile);
	~Words2() override;

	//Add words from the xdxf dictionary to the internal lists
	bool xdxfBuildDict(const std::string &dictFile, bool bUpdateDef, bool bXdxfDefOnly);

	//build the output dictionary from all loaded words
	bool filterGameWords();

	// iterate over the current dictionary and assign all match words to each 'reword' word 
	bool prematch();

	bool load(const std::string &wordFile = "", 		//load a wordlist and exclude
				unsigned int rndSeed = 0,				//duplicates, too many etc
				unsigned int startAtWord = 0) override;

	bool save(std::string outFile, bool bPrematch);

	Words2 & operator = (const Words2 &w2)
	{
		// Check for self-assignment
		if (this != &w2)      // not same object, so add all of 'w2' to 'this'
		{
			this->Words::operator=(w2);	//call Words overload first

            _doc = 0;   //shouldn't copy open file handle

            _countXdxfWords = w2._countXdxfWords;
            _countXdxfSkipped = w2._countXdxfSkipped;
            _countXdxfMatched = w2._countXdxfMatched;
            _countXdxfMissing = w2._countXdxfMissing;
            _bAutoSkillUpd = w2._bAutoSkillUpd;

			for (int i = TARGET_MAX; i >= SHORTW_MIN; --i)
			{
				_wordSet[i] = w2._wordSet[i];
			}
        }
		return *this;
	}
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
	Words2 operator+(const Words2 &other) const
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
				tWordSet temp;
				std::set_difference( _wordSet[i].begin(), _wordSet[i].end(),
									 w2._wordSet[i].begin(),w2._wordSet[i].end(),
									 std::inserter(temp, temp.begin()));
				temp.swap(_wordSet[i]);
			}

		}
		return *this;
	}
	Words2 operator-(const Words2 &other) const
	{
		return Words2(*this) -= other;	//call -= operator overload (as it's already there)
	}

	void setAutoSkillUpd(bool bOn = true) { _bAutoSkillUpd = bOn; }

protected:

	bool 			xdxfOpenDict(const std::string &dictFile);	//open generic xml file
	void 			xdxfCloseDict();
	TiXmlElement* 	xdxfFirstWord();
	TiXmlElement* 	xdxfNextWord(TiXmlElement* ar, std::string &word, std::string &def);

    int calcScrabbleSkillLevel(const std::string &word);
	void addWordsToSets();	//add to valid sets (one set per word length)
	int saveWordMap(FILE *& fp, const tWordMap &wmOrig, const tWordSet &wsFilt, bool bPrematch = false);

private:

	TiXmlDocument * _doc = nullptr;
	int		_countXdxfWords = 0;
	int		_countXdxfSkipped = 0;
	int		_countXdxfMatched = 0;
	int		_countXdxfMissing = 0;

	tWordSet _wordSet[TARGET_MAX+1];	//use 1..n for actual word length (as index) during rewordlist.txt build

    bool    _bAutoSkillUpd = false;     //update the word skill level with any non 0 value from any list
};

#endif //_WORDS2_H
