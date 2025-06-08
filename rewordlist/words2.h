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
	bool trialFilterByCount(int trialWordCount);
	bool trialFilterByFile(const std::string& trialWordFile);

	// iterate over the current dictionary and assign all match words to each 'reword' word 
	bool prematch();

	bool rejectDefinition(const DictWord& dictWord) override;

	bool load(const std::string &wordFile = "", 		//load a wordlist and exclude
				unsigned int rndSeed = 0,				//duplicates, too many etc
				unsigned int startAtWord = 0) override;

	bool save(std::string outFile, bool bPrematch);

	Words2 & operator = (const Words2 &w2);
	Words2 & operator += (const Words2 &w2);		// add all the important Word2 member vars
	Words2 & operator += (const tWordSet &ws);		// add just a word list/set
	Words2 operator+(const Words2 &other) const;

	Words2 & operator -= (const Words2 &w2);
	Words2 operator-(const Words2 &other) const;

	[[nodiscard]] tWordSet getWordSet() const;

	void setAutoSkillUpd(bool bOn = true) { _bAutoSkillUpd = bOn; }
	void setDefinitionExcl(const tWordSet& defExcl, const tWordSet& incWords) { _definitionExclSet = defExcl; _allIncludeWords = incWords; };

protected:

	bool 			xdxfOpenDict(const std::string &dictFile);	//open generic xml file
	void 			xdxfCloseDict();
	[[nodiscard]] TiXmlElement* 	xdxfFirstWord() const;
	[[nodiscard]] TiXmlElement* 	xdxfNextWord(TiXmlElement* ar, std::string &word, std::string &def);

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

	tWordSet _definitionExclSet;		// list of words to check definitions for and exclude words if found (e.g. "abbr." "prefix.")
	tWordSet _allIncludeWords;			// list of forced include words (used in definition exclusion tests)

    bool    _bAutoSkillUpd = false;     //update the word skill level with any non 0 value from any list
};

#endif //_WORDS2_H
