//words2.h

#ifndef _WORDS2_H
#define _WORDS2_H

#include <deque>
#include <set>
#include <map>
#include <vector>
#include <string>
#include "stdio.h"

#include "../reword/words.h"
#include "./tinyxml/tinyxml.h"


class Words2 : public Words
{
public:
	Words2();
	~Words2();

	bool openXmlDict(std::string dictFile);	//open generic xml file
	TiXmlElement* firstXdxfWord();
	TiXmlElement* nextXdxfWord(TiXmlElement* ar, std::string &word, std::string &def);
	bool matchXdxfDict(bool bUpdateDef);

	//build the output dictionary just from an input xdxf file
	bool buildXdxfDict(std::string outFile, std::string dictFile);

	//build the output dictionary from a wordlist and a xdxf dictionary
	bool filterOut(std::string outFile, const std::string &dictFile = "", bool bUpdateDef = false);

	int saveWordMap(FILE *& fp, tWordMap &wmOrig, tWordSet &wsFilt);
	bool save(std::string outFile);

protected:

	TiXmlDocument * _doc;
	int		_countXdxfWords;
	int		_countXdxfSkipped;
	int		_countXdxfMatched;
	int		_countXdxfMissing;

	tWordSet _ws6, _ws5, _ws4, _ws3;

};

#endif //_WORDS2_H
