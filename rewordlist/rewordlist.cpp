////////////////////////////////////////////////////////////////////
/*

File:			rewordlist.cpp

Description:	Defines the entry point for the rewordlist application.
				This command line app is used to build the wordlist file (rewordlist.txt)
				used in the reword game.

				See README.txt for more info

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			15 May 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.1		06.09.2006	First version
				0.2		22.10.2006	Upped maximum words from 7 to 8 per 6 letter
										word before cutoff. This increased reword.txt
										by about 500 to ~1700 6 letter words
				0.3		30.05.2007	Updated
									Added ability to parse and include xdxf dictionary definitions 
									to the words in the originating wordlist, or to create the 
									rewordlist.txt file directly from the xdxf dictionary.
									- Using a wordlist as the initial word file allows you to more easily 
									  control the words to be used in the game, and then apply any big enough 
									  xdxf dictionary to fill in the definitions.
									- Using purely the xdxf as the word source gives a more complete word list 
									  but may include words that are proper names etc, and not as good for a game
									Included a word difficulty rating, where 1=easy, 2=med, 3=hard, so each 
										difficulty level can be tuned for say, different age groups. This 
										unfortunately must be manually entered for each word as it's a 
										personal setting. Only 6 letter words need to be set currently 
										as smaller words are not filtered using the level.
				0.4		08.11.2007	Added options to use exclusion and/or inclusion files of words 
										to always remove/insert, allowing different wordlists to be 
										tried without having to reedit each time to make sure certain words 
										are always removed or added.
						19.03.2008	Improved include/exclude code + upped max def to 2000 chars

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


#include <ios>
#include <iostream>
#include <string>

#include "../reword/platform.h"
#include "../reword/helpers.h"
#include "../reword/words.h"
#include "words2.h"

using namespace std;

//undefine sdl main definition if sdl has been included in a embedded include
//otherwise sdl defines main as SDL_Main and linker fails to find _main
#ifdef _SDL_H
#undef main
#endif
int main(int argc, char* argv[])
{
	bool bList = false, bDebug = false, bForceDef = false;
	string::size_type pos;
	std::string dictFile;
	std::string wordList, includeList, excludeList;
	bool bHelp = false;

	//v. simple loop to load cmd line args - in any order, 
	//but must be seperately 'dashed' ie. -l -f not -lf
	for (int i = 1; i< argc; ++i)
	{
		std::string arg = argv[i];
		if ("--help" == arg) 
		{ 
			bHelp = true;
			continue;
		}
		if ("-l" == arg) 
		{ 
			bList = true;
			continue;
		}
		if ("-d" == arg) 
		{ 
			bDebug = true;
			continue;
		}
		if ("-f" == arg) 
		{ 
			bForceDef = true;
			continue;
		}
		pos = arg.find_last_of(".");		//find the last period for file extension
		if (pos == string::npos) continue;	//unknown (doesnt end in a file extension)
		std::string ext(arg.substr(pos));
		if (".xdxf" == ext)
		{
			dictFile = arg;
			continue;
		}
		if (".txt" == ext)
		{
			if (strcasecmp(arg.c_str(), "rewordlist.txt") == 0)
			{
				std::cout << std::endl << "You cannot use the file rewordlist.txt as an input file" <<  std::endl;
				exit(0);
			}
			wordList = arg;
			continue;
		}
		if (".include" == ext)
		{
			includeList = arg;
			continue;
		}
		if (".exclude" == ext)
		{
			excludeList = arg;
			continue;
		}

		//else ignore anything else for now
	}

	if (!bHelp)
	{
		std::cout << "Working..." << std::endl;

		Words2 words2;
		words2.setList(bList);
		words2.setDebug(bDebug);
	
		std::string outFile("rewordlist.txt");

		bool bSave = false;
	
		if (wordList.length() == 0 && dictFile.length() > 0)
		{
			if (includeList.length() > 0)
			{
				Words includeWords(includeList);
				int iOrig = words2.size();
				words2 += includeWords;	//add any forced include words
				int iNew = words2.size();
				std::cout << "Added " << iNew-iOrig <<  " words using " << includeList << std::endl;
			}

			//just load and populate rewordlist.txt from xdxf
			bSave = words2.buildXdxfDict(outFile, dictFile);
		}
		else if (wordList.length() > 0)	//dictFile could be blank or valid
		{
			//load the named wordlist (with or without level and definition values)
			if (words2.load(wordList))	
			{
				if (includeList.length() > 0)
				{
					Words includeWords(includeList);
					int iOrig = words2.size();
					words2 += includeWords;	//add any forced include words
					int iNew = words2.size();
					std::cout << "Added " << iNew-iOrig <<  " words using " << includeList << std::endl;
				}

				//remove unwanted words (i.e. too long, too short, bad chars etc),
				//and try to add dict entries to output final rewordlist.txt 
				bSave = words2.filterOut(outFile, dictFile, bForceDef);
			}
		}
		else bHelp = true;

		if (!bHelp && bSave)
		{
			if (excludeList.length() > 0)
			{
				Words excludeWords(excludeList);
				int iOrig = words2.size();
				words2 -= excludeWords;	//remove any exclusion words
				int iNew = words2.size();
				std::cout << "Removed " << iOrig-iNew <<  " words using " << excludeList << std::endl;
			}

			//save it
			if (words2.save(outFile))
			{
				std::cout << std::endl << "Created \"" << outFile <<  "\" from ";
				if (wordList.length()) 
				{
					std::cout << wordList;
					if (dictFile.length()) std::cout << " and ";
				}
				if (dictFile.length()) std::cout << dictFile;
				std::cout << std::endl << "Place this in the data directory of the reword game" << std::endl;
			}
			else
				std::cout << std::endl << "Error: Unable to filter " << wordList << " into " << outFile << std::endl;
		}
	}

	if (bHelp)
	{
		std::cout << "Utility (version 0.4) to generate rewordlist.txt for the reword game." << std::endl
				<< "Useage:" << std::endl 
				<< "rewordlist [words.txt] [words.include] [words.exclude] [dictionary.xdxf] [-f] [-l] [-d]" << std::endl
				<< std::endl 
				<< "  Params:  " << std::endl
				<< "  words.txt is a simple one word per line wordlist, which may include |diff|def " << std::endl
				<< "        (omit, if creating rewordlist.txt directly from xdxf)" << std::endl
//				<< "        (Also, can be rewordlist.txt from a prev. run to add xdxf etc, see readme)" << std::endl
				<< "  words.include is a simple one word per line wordlist of words to force inclusion of.." << std::endl
				<< "  words.exclude is a simple one word per line wordlist of words to exclude" << std::endl
				<< "  dictionary.xdxf is the optional dictionary for word definitions" << std::endl
				<< "        (if used exclusively ,may contain unwanted real names etc)" << std::endl
				<< "  -f to force xdxf definition overwrite mode (replaces definition if new one found)" << std::endl
				<< "  -l to force listing (verbose) mode" << std::endl
				<< "  -d to force debug listing (messages) mode" << std::endl
				<< std::endl 
				<< "If just a xdxf file given, use that to create rewordlist.txt with definitions" << std::endl
				<< "If just a wordlist txt file given, create just a filtered rewordlist.txt" << std::endl
				<< "If a wordlist text file and a xdxf given, use both to create rewordlist.txt with definitions" << std::endl;
		return 0;
	}

	return 0;
}
