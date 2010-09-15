////////////////////////////////////////////////////////////////////
/*

File:			words2.cpp

Class impl:		Words2

Description:	Class derived from CWords to handle creation of wordlist file to include 
				optional xdxf dictionary entries.
				Uses TinyXml to parse the xdxf xml(ish) dictionary format. Dictionaries 
				can be obtained from http://xdxf.sourceforge.net/

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			15 May 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.3		15.05.2007	Created
				0.4		08.11.2007	Added include/exclude file names to filterOut() fn

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

#include "words2.h"
#include "../reword/helpers.h"	//string helpers etc

Words2::Words2() : _doc(0)
{
	_countXdxfWords = _countXdxfSkipped = _countXdxfMatched = _countXdxfMissing = 0;
}

Words2::~Words2()
{
	delete _doc;
}

bool Words2::openXmlDict(std::string dictFile)
{
	if (!dictFile.length()) return false;
	if (!_doc) 
		_doc = new TiXmlDocument(dictFile);

	if (_doc && !_doc->LoadFile())
	{
		std::cout << "RewordList: Unable load " << dictFile.c_str() << std::endl;
		return false;
	}
	return true;
}

//returns pointer to first <ar> node
TiXmlElement* Words2::firstXdxfWord()
{
	if (!_doc) return 0;

	TiXmlElement *root = _doc->FirstChildElement("xdxf");
	if (root)
	{
		return root->FirstChildElement("ar");
	}
	return 0;
}

//takes the current <ar> node pointer and gets the word and definition for it
//Then moves the pointer on to the next <ar> node (or 'end' if no more found) 
//
//Note, I find the formatting of the definition within the node bizzarre as the 
//definition can be split into several blocks, with nodes embedded with the text.
//Not the way I'd define a dictionary schema, but hey, its free and comprehensive
//so I can't really complain.
TiXmlElement* Words2::nextXdxfWord(TiXmlElement* ar, std::string &word, std::string &def)
{
	if (!_doc) return 0;

	TiXmlElement *tmpElement;
	TiXmlText *tmpText;

	TiXmlNode *child = 0;
	for( child = ar->FirstChild(); child; child = child->NextSibling() )
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
				pp_s::makeUpper(word);			//force to UPPER case (def tests against it, below)
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
			std::string prefix = def.substr(0,pos);	//may contain other chars, if so those words will stay 
			pp_s::makeUpper(prefix);
			if (word == prefix)
				def = def.substr(pos);

			if (def.length() > 2000)
			{
				def.erase(2000);
				def += "...";	//indicate it was cut short
			}
//			std::cout << word.c_str() << " - def: " << tmpstr << std::endl;

			break;	//out of for child loop as we now want next <ar> word and its def
		}
	}

	return ar->NextSiblingElement("ar");
}


bool Words2::matchXdxfDict(bool bUpdateDef)
{
	if (!_doc) return false;

	bool bFound = false;
	std::string word;
	std::string def;
	tWordMap::iterator it;
	TiXmlElement *ar = firstXdxfWord();
	while (ar)
	{
		if ((ar = nextXdxfWord(ar, word, def)))
		{
			bFound = false;

			//match the word with one we already validated from our 
			//word list and add the dictionary entry to it
			pp_s::makeAlpha(word);			//strip any non a-z chars
			it = _mapAll.find(word);
			if (it != _mapAll.end())			//found the word
			{
				bFound = true;
				if (bUpdateDef || (*it).second._description.length()==0)	//overwrite or blank
				{
					if (_bDebug) std::cout << "DEBUG: Xdxf " << word.c_str() << " : def = " << def.c_str() << std::endl;

					++_countXdxfMatched;
					(*it).second._description = def;
				}
				else ++_countXdxfSkipped;
			}
			
			if (word.length() < 6)
			{
				//Q&D!
				//if the shorter word doesnt end in 'S' try it again, with an 'S'
				//as plurals not always in the dictionary file, just the singular,
				//but they may well be in the originating wordlist.
				if (word[word.length()-1] != 'S')
				{
					word += 'S';

					it = _mapAll.find(word);
					if (it != _mapAll.end())			//found the word
					{
						bFound = true;
						if (bUpdateDef || (*it).second._description.length()==0)	//overwrite or blank
						{
							if (_bDebug) std::cout << "DEBUG: Xdxf " << word.c_str() << " : def = " << def.c_str() << std::endl;

							++_countXdxfMatched;
							(*it).second._description = def;

							if (_bList) std::cout << "Found from singular?: " << word << std::endl;
						}
						else ++_countXdxfSkipped;
						continue;	//next ar
					}
				}
			}

			if (!bFound) ++_countXdxfMissing;
		}
	}

	return true;
}

//build the word list from the xdxf dictionary without using the wordlist
//Needs 2 passes to build valid 6 letter words first, then those 6 letter words 
//with 3, 4, or 5 letter words within them that will be saved as the result file.
bool Words2::buildXdxfDict(std::string outFile, std::string dictFile)
{
	if (_doc) return false;	//already open!

	//scan any xdxf format dictionary file and assign the description to the 
	//same word in our list of acceptable words for the game.
	if (openXmlDict(dictFile))
	{
		//first, load all words into memory

		tWordSet dictSet;	//to remove duplicates... discarded after load()
		std::pair<tWordSet::const_iterator, bool> dictPair;
		DictWord dictWord;
		
		std::string word;
		std::string def;
		TiXmlElement *ar = firstXdxfWord();
		while (ar)
		{
			if ((ar = nextXdxfWord(ar, word, def)))
			{
				_total++;
				if (NULL != strchr(word.c_str(), '\''))  //ignore words with apostrophies
				{
					if (_bDebug) std::cout << "DEBUG: Ignore (\') " << word.c_str() << std::endl;
					_ignored++;
					continue;
				}

				pp_s::makeAlpha(word);			//strip any non a-z chars
				pp_s::makeUpper(word);			//force to UPPER case

				if (_bDebug) std::cout << "DEBUG: Line " << _total << ": " << word.c_str() << std::endl;

				dictPair = dictSet.insert(word);
				if (!dictPair.second)	//not inserted so is a duplicate word
				{
					if (_bDebug) std::cout << "DEBUG: Line " << _total << " Duplicate: " << word.c_str() << std::endl;
					_ignored++;
					continue;
				}

				dictWord._word = word;
				dictWord._description = def;
				dictWord._level = 0;

				//add word to dict
				if (_mapAll.find(word) == _mapAll.end())			//not found
				{
					if (_bDebug) std::cout << "DEBUG: Adding " << word.c_str() << " : def = " << def.c_str() << std::endl;

					_mapAll[word] = dictWord;		//so insert it in the ALL word map
					if (6 == word.length())			//is a 6 letter word 
						_vecTarget.push_back(word);	//so also add to valid 6 letter word vector used for nextWord()
				}
			}
		}

		//now again, to match 3, 4, 5 letter words

		_ws3.clear(); _ws4.clear(); _ws5.clear(); _ws6.clear();

		tWordVect::const_iterator it = _vecTarget.begin();
		while (it != _vecTarget.end())
		{
			clearCurrentWord();
			if (checkCurrentWordTarget(it))
			{
				//add to valid words

				tWordsInTarget::iterator it2 = _wordsInTarget.begin();
				while (it2 != _wordsInTarget.end())
				{
					switch ((*it2).first.length())
					{
					case 3:_ws3.insert((*it2).first);break;
					case 4:_ws4.insert((*it2).first);break;
					case 5:_ws5.insert((*it2).first);break;
					case 6:_ws6.insert((*it2).first);break;
					default:break;
					}
					++it2;
				}
			}
			++it;
		}

		std::cout << std::endl << "From Xdxf dict : " << dictFile << std::endl;

		std::cout << std::endl << "      Loaded   : " << _total << " words" << std::endl;
		std::cout << std::endl << "      Rejected : " << _ignored << " words" << std::endl;

		std::cout << std::endl << "      Loaded   : " << _countXdxfWords << " words with definitions" << std::endl;
		std::cout << std::endl << "      Skipped  : " << _countXdxfSkipped << " words due to blank definition or already defined" << std::endl;
		std::cout << std::endl << "      Matched  : " << _countXdxfMatched << " definitions to wordlist" << std::endl;
		std::cout << std::endl << "      Missing  : " << _countXdxfMissing << " words in dictionay but not in filtered wordlist" << std::endl;

		return true;
	}
	std::cout << "Failed to open rewordlist output file: " << outFile << std::endl;
	return false;
}


//Filter the loaded wordlist that has been processed to remove duplicates or words with too 
//many or too few letters etc and save it back out to the named file. 
//This function is primarily used to pre filter the wordlist file so that the game would 
//have no problem loading it and could process each word without having to worry about said duplicates etc.
//This makes it easier to build level and description markers into the word list file 
//without wasting time on words that will eventually be excluded by the game anyway. 
bool Words2::filterOut(std::string outFile, const std::string &dictFile, bool bUpdateDef)
{
	if (!_total)
	{
		std::cout << "Nothing to filter or save" << std::endl;
		return false;	//nothing loaded
	}

	//now we know we can open the output file, so proceed - which may take a while...
	std::cout << "Filtering ... " << _vecTarget.size() << std::endl;

	_ws3.clear(); _ws4.clear(); _ws5.clear(); _ws6.clear();

	tWordVect::const_iterator it = _vecTarget.begin();
	while (it != _vecTarget.end())
	{
		clearCurrentWord();
		if (checkCurrentWordTarget(it))
		{
			//add to valid words
			tWordsInTarget::iterator it2 = _wordsInTarget.begin();
			while (it2 != _wordsInTarget.end())
			{
				switch ((*it2).first.length())
				{
				case 3:_ws3.insert((*it2).first);break;
				case 4:_ws4.insert((*it2).first);break;
				case 5:_ws5.insert((*it2).first);break;
				case 6:_ws6.insert((*it2).first);break;
				default:break;
				}
				++it2;
			}
		}
		++it;
	}


	//scan any (optional) xdxf format dictionary file and assign the description 
	//to the same word in our list of acceptable words for the game.
	if (openXmlDict(dictFile))
	{
		//search dictionary for definitions and update if needed
		matchXdxfDict(bUpdateDef);
	}

	std::cout << "From wordlist  : " << _wordFile << std::endl;
	std::cout << "     Loaded    : " << _total << " words" << std::endl;
	std::cout << "     Rejected  : " << _ignored << " words" << std::endl;

	if (dictFile.length())
	{
		std::cout << std::endl;
		std::cout << "From Xdxf dict : " << dictFile << std::endl;
		std::cout << "     Loaded    : " << _countXdxfWords << " words with definitions" << std::endl;
		std::cout << "     Skipped   : " << _countXdxfSkipped << " words due to blank definition or already defined" << std::endl;
		std::cout << "     Matched   : " << _countXdxfMatched << " definitions to wordlist" << std::endl;
		std::cout << "     Missing   : " << _countXdxfMissing << " words in dictionay but not in filtered wordlist" << std::endl;
	}

	return true;
}

//output the actual word list to file
//The wordMap is the original list and the wordSet is the filtered list that 
//is to be output.
int Words2::saveWordMap(FILE *& fp, tWordMap &wmOrig, tWordSet &wsFilt)
{
	int count = 0;
	for (tWordMap::const_iterator wrd = wmOrig.begin (); wrd != wmOrig.end (); ++wrd)
	{
		if (wsFilt.find((*wrd).second._word) != wsFilt.end())
		{
			//found dictionary word in filtered set, so save it
			//build word line "word|level|description"
			//level may be 0 if not explicitly specified in originally loaded word list file
			//description may be blank, in which case the pipe (|) divider need not be added
			fprintf(fp, "%s|%d", (*wrd).second._word.c_str(), (*wrd).second._level);
			if ((*wrd).second._description.length())
				fprintf(fp, "|%s",(*wrd).second._description.c_str());
			fprintf(fp, "\n");
			++count;
		}
	}
	return count;
}

bool Words2::save(std::string outFile)
{
	FILE *fp;
	if (!outFile.length()) outFile = _wordFile;	//save back out to same file loaded

	int iout=0;
	if ((fp = fopen(outFile.c_str(), "w+")))  //create output file even if exists
	{
		//now save filtered dictionary...

		if (_bList) std::cout << "Writing..." << outFile <<std::endl;

		//duplicated looping through _mapAll to find wsN, but orders output as 6 then 5 then 4 then 3.
		//which is easier to read and maintain. 
		iout += saveWordMap(fp, _mapAll, _ws6);
		iout += saveWordMap(fp, _mapAll, _ws5);
		iout += saveWordMap(fp, _mapAll, _ws4);
		iout += saveWordMap(fp, _mapAll, _ws3);

		fclose(fp);

		std::cout << std::endl << "Output: " << iout << " filtered words " << std::endl;
	}
	else
	{
		std::cout << "Failed to open shortwordlist output file: " << outFile << std::endl;
		return false;
	}
	return true;
}