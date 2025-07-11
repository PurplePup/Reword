////////////////////////////////////////////////////////////////////
/*

File:			words2.cpp

Class impl:		Words2

Description:	Class derived from Words to handle creation of wordlist file by rewordlist
				application. Allows optional xdxf dictionary entries.
				Uses TinyXml to parse the xdxf xml(ish) dictionary format. Dictionaries
				can be obtained from http://xdxf.sourceforge.net/

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			15 May 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3		15.05.2007	Created
				0.4		08.11.2007	Added include/exclude file names to filterGameWords() fn
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


#include <fstream>
#include <ios>
#include <iostream>
#include <stdio.h>
#include <regex>
#include <algorithm>
#include <array>
#include <random>

#include "words2.h"
#include "../reword/helpers.h"	//string helpers etc


Words2::Words2()
{
	_countXdxfWords = _countXdxfSkipped = _countXdxfMatched = _countXdxfMissing = 0;
}

Words2::Words2(const std::string& wordFile)
{
	load(wordFile);	//calls reset() etc
}

Words2::~Words2()
{
	xdxfCloseDict();
}

Words2 & Words2::operator = (const Words2 &w2)
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
Words2 & Words2::operator += (const Words2 &w2)
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
Words2 & Words2::operator += (const tWordSet &ws)
{
	for (auto const& w: ws)
	{
		this->_mapAll.emplace(w, DictWord());

		if (w.length() >= SHORTW_MIN && w.length() <= TARGET_MAX)
		{
			_wordSet[w.length()].insert(w);
		}
	}
	return *this;
}
Words2 Words2::operator+(const Words2 &other) const
{
	return Words2(*this) += other;	//call += operator overload (as it's already there)
}

Words2 & Words2::operator -= (const Words2 &w2)
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
Words2 Words2::operator-(const Words2 &other) const
{
	return Words2(*this) -= other;	//call -= operator overload (as it's already there)
}

bool Words2::xdxfOpenDict(const std::string& dictFile)
{
	if (!dictFile.length()) return false;

	xdxfCloseDict();	//if already open, close it
	_doc = new TiXmlDocument(dictFile);

	if (_doc && !_doc->LoadFile())
	{
		std::cout << "RewordList: Unable to load " << dictFile.c_str() << std::endl;
		return false;
	}
	return true;
}

void Words2::xdxfCloseDict()
{
	delete _doc;
	_doc = nullptr;
}

//returns pointer to first <ar> node
TiXmlElement* Words2::xdxfFirstWord() const
{
	if (!_doc) return nullptr;

	if (TiXmlElement* root = _doc->FirstChildElement("xdxf"))
	{
		return root->FirstChildElement("ar");
	}
	return nullptr;
}

//takes the current <ar> node pointer and gets the word and definition for it
//Then moves the pointer on to the next <ar> node (or 'end' if no more found)
//
//Note, I find the formatting of the definition within the node bizzarre as the
//definition can be split into several blocks, with nodes embedded with the text.
//Not the way I'd define a dictionary schema, but hey, its free and comprehensive
//so I can't really complain.
TiXmlElement* Words2::xdxfNextWord(TiXmlElement* ar, std::string& word, std::string& def)
{
	if (!_doc) return nullptr;

	TiXmlElement* tmpElement = nullptr;
	TiXmlText* tmpText = nullptr;

	TiXmlNode* child = nullptr;
	for (child = ar->FirstChild(); child; child = child->NextSibling())
	{
		//		std::string text = child->Value();
		//		std::cout << "<k>" << text.c_str() << "</k> : " << std::endl;

		if (child->Type() == TiXmlNode::ELEMENT)
		{
			tmpElement = child->ToElement();
			if (tmpElement->ValueStr() == "k")
			{
				++_countXdxfWords;
				word = tmpElement->GetText();	//get the <k>WORD</k>
				pptxt::makeUpper(word);			//force to UPPER case (def tests against it, below)
			}
			//else ignore; dont care about <b> or <c> etc
		}
		else if (word.length() && child->Type() == TiXmlNode::TEXT)
		{
			tmpText = child->ToText();
			def = tmpText->Value();	//get the text (NOT in a <></> tag, wtf?

			//check if the def starts with the dictionary word.
			//Some xdxf dictionary defs I've seen do something like:
			//"abbacy n : the jurisdiction or office of an abbot"
			//where the "abbacy " part is not required for the game,
			//as it's already shown so try and remove it.
			std::string::size_type pos = def.find_first_of(" ");
			std::string prefix = def.substr(0, pos);	//may contain other chars, if so those words will stay
			pptxt::makeUpper(prefix);
			if (word == prefix)
				def = def.substr(pos);

			if (def.length() > MAX_REWORD_DESCRIPTION)
			{
				def.erase(MAX_REWORD_DESCRIPTION);
				def += "...";	//indicate it was cut short
			}
			//			std::cout << word.c_str() << " - def: " << tmpstr << std::endl;

			break;	//out of for child loop as we now want next <ar> word and its def
		}
	}

	return ar->NextSiblingElement("ar");
}

//build the word list from the xdxf dictionary without using the wordlist
//Needs 2 processes to build valid 6to8 letter words first, then those 6to8 letter words
//with shorter letter words within them that will be saved as the result file.
//Don't reset values before starting as .include files may have been loaded
bool Words2::xdxfBuildDict(const std::string& dictFile, bool bUpdateDef, bool bXdxfDefOnly)
{
	//scan any xdxf format dictionary file and assign the description to the
	//same word in our list of acceptable words for the game.
	if (xdxfOpenDict(dictFile))
	{
		//first, load all words into memory

		long lDefsUpdated(0);
		tWordSet dictSet;	//to remove duplicates... discarded after load()
		std::pair<tWordSet::const_iterator, bool> dictPair;
		DictWord dictWord;

		std::string word;
		std::string def;
		TiXmlElement* ar = xdxfFirstWord();
		while (ar)
		{
			if ((ar = xdxfNextWord(ar, word, def)))
			{
				_stats._total++;
				pptxt::makeAlpha(word);			//strip any non a-z chars
				pptxt::makeUpper(word);			//force to UPPER case

				//if we're only updating existing words, with definitions from the xdxf
				//dictionary, don't add words just find them and poss update the definition.
				if (bXdxfDefOnly)
				{
					tWordMap::iterator it = _mapAll.find(word);
					if (it != _mapAll.end())    //found?
					{
						if (bUpdateDef || it->second._description.empty())
						{
							it->second._description = def;
							++lDefsUpdated;
						}
					}
					continue;
				}

				if (rejectWord(word))
				{
					_stats._ignored++;
					continue;
				}

				if (_bDebug) std::cout << "DEBUG: Line " << _stats._total << ": " << word.c_str() << std::endl;

				dictPair = dictSet.insert(word);
				if (!dictPair.second)	//not inserted so is a duplicate word from the input (xdxf) file
				{
					if (_bDebug) std::cout << "DEBUG: Line " << _stats._total << " Duplicate: " << word.c_str() << std::endl;
					_stats._ignored++;
					continue;
				}

				//add/update word in dict
				const bool bExists = (_mapAll.find(word) != _mapAll.end());

				dictWord._word = word;
				dictWord._description = def;
				dictWord._level = 0;    //level only defined in .txt files, not .xdxf

				_mapAll[word] = dictWord;			//so insert/amend it in the ALL word map
				if (!bExists && (word.length() >= TARGET_MIN && word.length() <= TARGET_MAX))
				{
					if (_bDebug) std::cout << "DEBUG: Adding " << word.c_str() << " : def = " << def.c_str() << std::endl;
					_vecTarget.push_back(word);		//so also add to valid target word vector used for nextWord()
				}
			}
		}

		if (bXdxfDefOnly)
		{
			std::cout << "Updated " << lDefsUpdated << " words from Xdxf file " << dictFile << std::endl;
		}

		return true;
	}
	return false;
}

//build the word sets from the full list using the target list as the source
void Words2::addWordsToSets()
{
	const int iMod = 200;
	int iCount = _vecTarget.size();
	int iDisplayMod = iCount / std::min(iMod, iCount+1);

	std::cout << std::endl << std::unitbuf; // enable automatic flushing
	std::cout << "Filtering " << _vecTarget.size() << " targets..." << std::endl;

	for (auto const& target : _vecTarget)
	{
		if (iCount > iMod && !(iCount-- % iDisplayMod))
			std::cout << "\r" << "Remaining: " << iCount << "       ";

		//make sure it's a word length we support (say 3..8)
		if (target.length() >= SHORTW_MIN && target.length() <= TARGET_MAX)
		{
			clearCurrentWord();
			if (checkCurrentWordTarget(target))	//finds shorter words in target word
			{
				// // check if any exclusion by definition words also exclude the word
				// if (!_definitionExclSet.empty())
				// {
				// 	auto p = _mapAll.find(target);
				// 	if (p != _mapAll.end())
				// 	{
				// 		for (auto w : _definitionExclSet)
				// 		{
				// 			if (p->second._description.find(w) != std::string::npos)
				// 			{
				//
				// 			}
				//
				// 		}
				// 	}
				// }

				//add to valid words - if within word length size required
				for (auto const& word : _wordsInTarget)
				{
					const int wordLen = word.first.length();
					if (wordLen >= SHORTW_MIN && wordLen <= TARGET_MAX)
					{
						_wordSet[wordLen].insert(word.first);
					}
				}
			}
		}
	}
	std::cout << "\rFiltering done        " << std::endl << std::nounitbuf;
}

//Filter the loaded wordlists that has been processed to remove duplicates or words with too
//many or too few letters etc and get it ready to save back out to the named file.
//Must be called before save is used.
bool Words2::filterGameWords()
{
	//now we know we can open the output file, so proceed - which may take a while...
	addWordsToSets();
	return true;
}

// Find n target words and include them and all associative words and return the list
bool Words2::trialFilterByCount(int trialWordCount)
{
	int filtered = 0;
	int countdown = trialWordCount;

	for (auto set : _wordSet)
		set.clear();

	std::random_device rd;
	std::shuffle(_vecTarget.begin(), _vecTarget.end(), rd);

	for (const auto& target : _vecTarget)
	{
		if (countdown <= 0)
			break;

		if (target.length() < TARGET_MIN)
			continue;

		countdown--;

		clearCurrentWord();
		findWordsInWordTarget(_mapAll, target.c_str());	//side effect - fills _nWords[] & _wordsInTarget
		for (auto const& [word, found] : _wordsInTarget)
		{
			_wordSet[word.length()].insert(word);
			filtered++;
		}
	}
	return filtered > 0;
}
// Find target words from the text file and include thenm and all associative words and return the list
bool Words2::trialFilterByFile(const std::string& trialWordFile)
{
	return false;
}



// Previously findWordsInWordTarget() was called per target word during play, but due to 
// platforms with limited processing power, we will call it on all target words and output 
// these found words in the loadable dictionary itself rather than calculate live.
// new dict line format:
// TARGET | level | WORD[,WORD,WORD,...] | description
// Dictionary file must have extension .rw2 to distinguish the new line format
bool Words2::prematch() 
{
	std::cout << std::endl << std::unitbuf; // enable automatic flushing
	std::cout << "Prematching " << _vecTarget.size() << " targets..." << std::endl;

	int iCount = _vecTarget.size();
	int iDisplayMod = iCount / 200;

	//#pragma omp parallel for
	for (const auto& target : _vecTarget)
	{
		if (target.length() < TARGET_MIN)
			continue;

		clearCurrentWord();
		findWordsInWordTarget(_mapAll, target.c_str());	//side effect - fills _nWords[] & _wordsInTarget

		if (_bDebug)
			std::cout << "Words in target : " << target << std::endl;
		else
		{
			if (!(iCount-- % iDisplayMod))
				std::cout << "\r" << "Remaining: " << iCount << "       ";
		}

		// copy map of found words into sorted vector of shortest to longest (ascending)
		std::vector<std::string> found_words;
		found_words.reserve(_wordsInTarget.size());
		std::transform(_wordsInTarget.begin(), _wordsInTarget.end(), back_inserter(found_words),
			[](std::pair<std::string, int> const& pair)
			{
				return pair.first;
			});
		// Use comparator tuple to test size first then string content
		auto comp = [](const std::string& s) { return std::make_tuple(s.size(), std::ref(s)); };
		std::sort(found_words.begin(), found_words.end(), 
			[comp](const std::string& s1, const std::string& s2) -> bool
			{
				return comp(s1) < comp(s2);
			});
		auto wrd = _mapAll.find(target);
		if (wrd != _mapAll.end())
		{
			wrd->second._prematch = found_words;
		}

		if (_bDebug)
		{
			std::copy(found_words.begin(), found_words.end(), std::ostream_iterator< std::string >(std::cout, ","));
			std::cout << std::endl;
		}
	}
	std::cout << "\rPrematching done        " << std::endl << std::nounitbuf;

	return true;
}


//Determine a 1,2,3 (easy/med/hard) score for the word passed in.
//Using the scrabble letter scoring system, each word is ranked and
//then given a 1,2,3 score.
//Common letter groups such as ING or TH that make detecting the word
//easier are removed leaving a word 'stem' before the word is scored.
/*
English-language editions of Scrabble contain 100 letter tiles, in the following distribution:

	2 blank tiles (scoring 0 points)
	1 point: E ×12, A ×9, I ×9, O ×8, N ×6, R ×6, T ×6, L ×4, S ×4, U ×4
	2 points: D ×4, G ×3
	3 points: B ×2, C ×2, M ×2, P ×2
	4 points: F ×2, H ×2, V ×2, W ×2, Y ×2
	5 points: K ×1
	8 points: J ×1, X ×1
	10 points: Q ×1, Z ×1


Digraph and Trigraph words to remove and retain stem word:
BLE,
CH, CK, CI,
IGH, ING,
OUS,
QU,
RH,
SH, SC, SCH,
TH,
WH, WR

EE, OO, NN, SS, ...    any double letter ?

*/
#define SCORE_EASY_THRESHOLD    7
#define SCORE_MED_THRESHOLD     12
int Words2::calcScrabbleSkillLevel(const std::string& word)
{
	if (word.length() < TARGET_MIN || word.length() > TARGET_MAX)
		return 0;   //only 6...8 currently scored

	//remove digraphs and trigraphs
	std::regex re(R"((BLE|CH|CK|CI|IGH|ING|OUS|QU|RH|SH|SCH|SC|TH|WH|WR|BB|CC|DD|EE|FF|GG|LL|MM|NN|OO|PP|RR|SS|TT|UU|WW|ZZ)*)");
	std::string out = std::regex_replace(word, re, "");

    //**English** language Scrabble letter scoring
	const std::array<int, 26> scores = { 1,3,3,2,1,4,2,4,1,8,5,1,3,1,1,3,10,1,1,1,1,4,4,8,4,10 };
    //count up the letter score
    int iTotal(0);
    for (int i = 0; i < static_cast<int>(out.length()); ++i) iTotal+=scores[out[i]-65];   //A-Z
    //gives a score between 6 (smalest re-word of all 1's) and 80 (largest 8 letter re-word of all 10's)

    //need to fine tune the distribution and therfore thresholds
    const int score = (iTotal < SCORE_EASY_THRESHOLD)? 1 : (iTotal < SCORE_MED_THRESHOLD) ? 2 : 3;
    _stats._countScore[iTotal]++;
    _stats._countLevels[score-1]++;

	if (_bDebug)
		std::cout << "Scrabble: '" << word << "' -> '" << out << "' score : " << iTotal << " = " << score << std::endl;

	return score;
}

// called from base class load() as extra exclusion list check as it needs to be
// before the excluded word is added to the internal maps.
// Only exists if called from rewordlist and Words2 calls load()
bool Words2::rejectDefinition(const DictWord& dictWord)
{
//	return std::any_of(_definitionExclSet.begin(), _definitionExclSet.end(),
//		[&strDef](const std::string& text){ return strDef.find(text) != std::string::npos; } );

	for (auto const& text : _definitionExclSet)
	{
		if (dictWord._description.find(text) != std::string::npos)
		{
			//check if the word is still to be included using the .include files
			if (_allIncludeWords.find(dictWord._word) != _allIncludeWords.end())
			{
				std::cout << "DEBUG Rejected " << dictWord._word << " exclusion - is in .include file" << std::endl;
				continue;
			}
			std::cout << "DEBUG Exclude " << dictWord._word << " for '" << text << "' in : " << dictWord._description << std::endl;
			return true;
		}
	}
	return false;
}

//load the file using the base class, then check if wee need to generate scrabble type skill
//level word weighting to work out automatic easy/med/hard difficulty settings for the words
//loaded.
bool Words2::load(
	const std::string& wordFile, 		//load a wordlist and exclude
	unsigned int rndSeed,			    //duplicates, too many etc
	unsigned int startAtWord)
{
	const bool bOk = Words::load(wordFile, rndSeed, startAtWord);
	if (bOk)
	{
		if (_bAutoSkillUpd)
		{
			std::cout << "Auto skill difficulty using Scrabble letter values ... " << std::endl;
			for (auto& wrd : _mapAll)
			{
				wrd.second._level = calcScrabbleSkillLevel(wrd.first);
			}
		}

		// sort and iterate and assign final output indexes from smallest to largest
		std::cout << "Indexing words..." << std::endl;
		std::vector<std::string> indexList;	// fill list of words then sort smallest->largest
		for (const auto& w : _mapAll)
		{
			indexList.push_back(w.first);
		}
		// Use comparator tuple to test size first then string content
		auto comp = [](const std::string& s) { return std::make_tuple(s.size(), std::ref(s)); };
		std::sort(indexList.begin(), indexList.end(), 
			[comp](const std::string& s1, const std::string& s2) -> bool
			{
				return comp(s1) < comp(s2);
			});
		int index = 0;
		for (const auto& w : indexList)
		{
			auto it = _mapAll.find(w);
			if (it != _mapAll.end())
			{
				it->second._index = index;
				//std::cout << "DEBUG " << it->first << "=" << w << ":" << it->second._index << std::endl;
			}
			++index;
		}
	}

	return bOk;
}

//output the actual word list to file
//The wordMap is the original list and the wordSet is the filtered list that
//is to be output. Called from save() for each set of same length words.
int Words2::saveWordMap(FILE *& fp, const tWordMap &wmOrig, const tWordSet &wsFilt, bool bPrematch)
{
	int count = 0;

	for (auto const & filtWord : wsFilt)
	{
		auto w = wmOrig.find(filtWord);
		if (w == wmOrig.end())
			continue;

		auto const & wrd = (*w).second;
		//found dictionary word in filtered set, so save it
		//build word line "word|level|prematch|description"
		// e.g. "BAMBOO|0|BOB,BOMB,...|n. 1 a mainly tropical giant woody grass of the subfamily Bambusidae..."
		//level may be 0 if not explicitly specified in originally loaded word list file
		//description may be blank, in which case the pipe (|) divider need not be added

        //level only defined in .txt files, not .xdxf, unles -s used to auto scrabble score
        const int level = _bAutoSkillUpd ? calcScrabbleSkillLevel(wrd._word) : wrd._level;

		fprintf(fp, "%s|%d|", wrd._word.c_str(), level);
			
		if (bPrematch)
		{
			// Save the position of the 
			std::string prematch;
			for (const auto & word : wrd._prematch)
			{
				if (word != wrd._word)
				{
					const auto ind = _mapAll.find(word);	//should NEVER fail to find a word
					prematch += (prematch.length()?",":"") + std::to_string(ind->second._index);
				}
			}
			fprintf(fp, "%s|", prematch.c_str());
		}
		if (wrd._description.length())
			fprintf(fp, "%s",wrd._description.c_str());
		fprintf(fp, "\n");
		++count;
	}
	return count;
}

bool Words2::save(std::string outFile, bool bPrematch)
{
	if (!outFile.length()) outFile = _wordFile;	//save back out to same file loaded

	FILE *fp = fopen(outFile.c_str(), "w+"); //create output file even if exists
	if (fp)
	{
		int iout = 0;
		int itotal = 0;
		//now save filtered dictionary...
		if (_bList) std::cout << "Writing..." << outFile << std::endl;

		if (bPrematch)
		{
			//loop through each word set and use _mapAll to output 3.. 4.. 5.. N.
			//with full prematch list and description and level value.
			for (int i = SHORTW_MIN; i <= TARGET_MAX; ++i)
			{
				iout = saveWordMap(fp, _mapAll, _wordSet[i], true);
				itotal += iout;
				if (_bList) std::cout << "Saved: " << iout << " " << i << " prematched letter filtered words" << std::endl;
			}
		}
		else
		{
			//loop through each word set and use _mapAll to output N.. 6.. 5.. 4.. 3.
			//with full description and level value.
			for (int i = TARGET_MAX; i >= SHORTW_MIN; --i)
			{
				iout = saveWordMap(fp, _mapAll, _wordSet[i]);
				itotal += iout;
				if (_bList) std::cout << "Saved: " << iout << " " << i << " letter filtered words " << std::endl;
			}
		}

		fclose(fp);

		std::cout << std::endl << "Saved: " << itotal << " total filtered words " << std::endl;
		std::cout << std::endl << "Rejected: " << _stats._ignored << " total filtered words " << std::endl;

		if (_bAutoSkillUpd)
		{
			std::cout << "Auto-skill breakdown:" << std::endl;
			for (unsigned int as = 0; as < (sizeof(_stats._countScore) / sizeof(int)); ++as)
			{
				if (_stats._countScore[as])
					std::cout << "Score: " << as << " has " << _stats._countScore[as] << " words." << std::endl;

			}
			std::cout << std::endl;
			std::cout << "Level Easy: " << _stats._countLevels[0] << " words, threshold is " << SCORE_EASY_THRESHOLD << std::endl;
			std::cout << "Level Med : " << _stats._countLevels[1] << " words, threshold is " << SCORE_MED_THRESHOLD << std::endl;
			std::cout << "Level Hard: " << _stats._countLevels[2] << " words." << std::endl;
		}
	}
	else
	{
		std::cout << "Failed to open '" << outFile << "' shortwordlist output file - err:" << errno << std::endl;
		return false;
	}
	return true;
}

tWordSet Words2::getWordSet() const
{
	tWordSet ws;
	for (const auto &[fst, snd] : _mapAll)
	{
		ws.insert(fst);
	}
	return ws;
}
