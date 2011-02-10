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
#include <set>
#include <algorithm>
#include <iterator>

#include "words2.h"

using namespace std;

//undefine sdl main definition if sdl has been included in a embedded include
//otherwise sdl defines main as SDL_Main and linker fails to find _main
#ifdef _SDL_H
#undef main
#endif
int main(int argc, char* argv[])
{
	bool bList(false), bDebug(false), bForceDef(false), bXdxfDef(false), bSkillUpd(false);
	bool bHelp(true), bHelpForce(false);
	std::string::size_type pos;
	std::string includeList, excludeList;
	tWordSet xdxfFiles;
	tWordSet txtFiles;

	//v. simple loop to load cmd line args - in any order,
	//but must be seperately 'dashed' ie. -l -f not -lf
	for (int i = 1; i< argc; ++i)
	{
		std::string arg = argv[i];
		if ("--help" == arg)
		{
			bHelp = bHelpForce = true;  //show help page
			continue;
		}
		if ("-l" == arg)
		{
			bList = true;       //display more debug info
			continue;
		}
		if ("-d" == arg)
		{
			bDebug = true;      //display extra debug info
			continue;
		}
		if ("-f" == arg)
		{
			bForceDef = true;   //create/overwrite definitions from included inputs
			continue;
		}
		if ("-x" == arg)
		{
			bXdxfDef = true;	//use xdxf files passed in for definitions only
			continue;
		}
		if ("-s" == arg)
        {
            bSkillUpd = true;    //update word skill value on any word list input
            continue;
        }
		pos = arg.find_last_of(".");		//find the last period for file extension
		if (pos == string::npos) continue;	//unknown (doesnt end in a file extension)
		std::string ext(arg.substr(pos));

		//allow multiple imput .xdxf files
		if (".xdxf" == ext)
		{
			xdxfFiles.insert(arg);
			bHelp = false;	//valid input
			continue;
		}
		//allow multiple input .txt files
		if (".txt" == ext)
		{
			if (strcasecmp(arg.c_str(), "rewordlist.txt") == 0)
			{
				std::cout << std::endl << "You cannot use the file rewordlist.txt as an input file" <<  std::endl;
				exit(0);
			}
			txtFiles.insert(arg);
			bHelp = false;	//valid input
			continue;
		}
		//only allow a single .include file
		if (".include" == ext)
		{
			includeList = arg;
			bHelp = false;	//valid input
			continue;
		}
		//only allow a single .exclude file
		if (".exclude" == ext)
		{
			excludeList = arg;
			bHelp = false;	//valid input
			continue;
		}

		//else ignore anything else for now
	}
	if (bHelpForce) bHelp = true;	//overrides

	if (!bHelp)
	{
		std::cout << "Working..." << std::endl;

		Words2 finalWords;
		finalWords.setList(bList);
		finalWords.setDebug(bDebug);
		finalWords.setSkillUpd(bSkillUpd);

		std::string outFile("rewordlist.txt");

		bool bSave = false;

//if -x used, restrict xdxf files to match only .txt words

		if (includeList.length() > 0)
		{
			std::cout << "Adding include list file '" << includeList <<  "'" << std::endl;
			Words2 includeWords(includeList);
			int iOrig = finalWords.size();
			finalWords += includeWords;	//add any forced include words
			std::cout << "Added " << finalWords.size() - iOrig <<  " words using " << includeList << std::endl;
		}
		if (!txtFiles.empty())
		{
			//load the named wordlists (with or without level and definition values)
			//Must be done before xdxf dictionaries as xdxf contain possible definitions
			//for the words in the wordlist files.
			int iOrig = finalWords.size();
			tWordSet::const_iterator it_txt = txtFiles.begin();
			for ( ; it_txt != txtFiles.end(); ++it_txt)
			{
				std::cout << "Adding word list .txt file '" << *it_txt <<  "'" << std::endl;
				Words2 txtWords;
				if (txtWords.load(*it_txt))
				{
                    if (bList) std::cout << "Inserting " << txtWords.size() << " words for processing" << std::endl;
					finalWords += txtWords;	//insert into main list (no dups)
				}
			}
			std::cout << "Added " << finalWords.size() - iOrig <<  " words using text files" << std::endl;
		}
		if (!xdxfFiles.empty())
		{
			//load all the named xdxf dictionary/definition files
			int iOrig = finalWords.size();
			tWordSet::const_iterator it_xdxf = xdxfFiles.begin();
			for ( ; it_xdxf != xdxfFiles.end(); ++it_xdxf)
			{
				std::cout << "Adding .xdxf dictionary file '" << *it_xdxf <<  "'" << std::endl;
				Words2 xdxfWords;

                if (xdxfWords.xdxfBuildDict(*it_xdxf, bForceDef))
				{
                    if (bList) std::cout << "Inserting " << xdxfWords.size() << " words from xdxf file" << std::endl;
					finalWords += xdxfWords;	//insert into main list (no dups)
				}
			}
			std::cout << "Added " << finalWords.size() - iOrig <<  " words using xdxf dictionary files" << std::endl;
		}
        if (excludeList.length() > 0)
        {
            std::cout << "Removing words found in exclude list file '" << excludeList <<  "'" << std::endl;
            Words2 excludeWords(excludeList);
            int iOrig = finalWords.size();
            finalWords -= excludeWords;	//remove any exclusion words
            std::cout << "Removed " << finalWords.size() - iOrig <<  " words using " << excludeList << std::endl;
        }

        if (finalWords.size() == 0)
		{
			std::cout << "Nothing to output." << std::endl;
			bSave = false;
		}
		else bSave = true;

		if (!bHelp && bSave)
		{
            //now all list and xdxf words added internally, filter out words
            //not needed due to not found in bigger words etc
            finalWords.filterGameWords();

			//save it
			if (finalWords.save(outFile))
			{
				std::cout << std::endl << "Created '" << outFile <<  "'";// from ";
//				std::copy( txtFiles.begin(), txtFiles.end(), std::ostream_iterator< std::string >( std::cout, "," ) );
	//			std::copy( xdxfFiles.begin(), xdxfFiles.end(), std::ostream_iterator< std::string >( std::cout, "," ) );
				std::cout << std::endl << "Place this file in the data/words/ directory of the Reword game" << std::endl;
			}
			else
				std::cout << std::endl << "Error: Unable to filter word lists into " << outFile << std::endl;
		}
	}

	if (bHelp)
	{
		std::cout << "Utility (version 0.4) to generate rewordlist.txt for the reword game." << std::endl
				<< "Useage:" << std::endl
				<< "rewordlist [words.txt] [words.include] [words.exclude] [dictionary.xdxf|...] [-f] [-l] [-d] [-x]" << std::endl
				<< std::endl
				<< "  Params:  " << std::endl
				<< "  words.txt is a simple one word per line wordlist, which may include |diff|def " << std::endl
				<< "        (omit, if creating rewordlist.txt directly from xdxf)" << std::endl
//				<< "        (Also, can be rewordlist.txt from a prev. run to add xdxf etc, see readme)" << std::endl
				<< "  words.include is a simple one word per line wordlist of words to force inclusion of.." << std::endl
				<< "  words.exclude is a simple one word per line wordlist of words to exclude" << std::endl
				<< "  dict.xdxf is the optional dictionary for word definitions" << std::endl
				<< "        (if used exclusively ,may contain unwanted real names etc)" << std::endl
				<< "  -f to force xdxf definition overwrite mode (replaces definition if new one found)" << std::endl
				<< "  -l to force listing (verbose) mode" << std::endl
				<< "  -d to force debug listing (messages) mode" << std::endl
				<< "  -x to use .xdxf files for definitions only, else used to create .txt words" << std::endl
				<< std::endl
				<< "e.g." << std::endl
				<< "If just a xdxf file given, use that to create rewordlist.txt with definitions" << std::endl
				<< "If just a wordlist txt file given, create just a filtered rewordlist.txt" << std::endl
				<< "If a wordlist text file and a xdxf given, use both to create rewordlist.txt with definitions" << std::endl;
		return 0;
	}

	return 0;
}
