////////////////////////////////////////////////////////////////////
/*

File:			helpers.cpp

Description:	General helper functions
				not relying on game headers or other code (other than stl)

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			10 Sep 2010

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.5		10.10.2010	Created

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


#include <algorithm>	//for std::min and std::max (if used) amongst other things
#include <cctype>
#include <sstream>
#include <cstring> 		//for strcasecmp/stricmp etc
#include <vector>

//#include <iostream>
//#include <cerrno>

namespace pp_s
{
  
void makeUpper(std::string &s)
{
	std::transform(s.begin(),s.end(),s.begin(),toupper);
}

void makeLower(std::string &s)
{
	std::transform(s.begin(),s.end(),s.begin(),tolower);
}

void trimRight(std::string &s, const std::string &tokens)
{
	s.erase(s.find_last_not_of(tokens)+1);
}

void trimLeft(std::string &s, const std::string &tokens)
{
	s.erase(0, s.find_first_not_of(tokens));
}

void trim(std::string &s, const std::string &tokens)
{
	trimLeft(s, tokens);
	trimRight(s, tokens);
}

//strip any non alpha (A-Z) letters from the string passed in
void makeAlpha(std::string &s)
{
	std::stringstream sOut;
	for (int i=0; i<(int)s.length(); ++i)
		if (isalpha(s[i])) sOut << s[i];
	s = sOut.str();
}

bool endsWith(const std::string &str, const std::string &match, bool caseSensitive /*= false*/)
{
	if (str.length() < match.length()) return false;
	if (strcasecmp(str.substr(str.length()-match.length()).c_str(),match.c_str()) != 0) return false;
	return true;
}

//function to take a string description (eg instructions or word definition) and split it up into lines ready for display on screen
void buildTextPage(std::string &inStr,unsigned int nCharsPerLine, std::vector<std::string> &outVect)
{
	outVect.clear();

	//split string up into lines displayable on screen...
	unsigned int start = 0, lastBreak = 0, curr = 0;
	std::string line;
	while (1)
	{
		if (curr-start > nCharsPerLine || curr == (unsigned int)inStr.length()-1  || '\n' == inStr[curr])
		{
			//if no break found, just copy up to the MAX chars and cut off
			if (lastBreak == start)	lastBreak = start+nCharsPerLine;
			//if we are at end of string, make sure its all added
			if (curr == (unsigned int)inStr.length()-1)
				lastBreak = (unsigned int)inStr.length();
			if ('\n' == inStr[curr])
				lastBreak = curr;

			//copy from start to lastBreak as one line
			line = inStr.substr(start, lastBreak-start);
			outVect.push_back(line);	//add to list of lines
			start = lastBreak+1;
		}
		//find next break
		if (' ' == inStr[curr] || '\n' == inStr[curr]) lastBreak = curr;
		//are we completely done?
		if (++curr >= (unsigned int)inStr.length()) break;
	}
}

bool splitKeyValuePair(const std::string &line, std::string &key, std::string &value)
{
	//split a config file type entry "key=value" into its "key"/"value" parts
	std::string::size_type pos = line.find("=");
	if (pos != std::string::npos)
	{
		key = line.substr(0,pos);
		if (pos < line.length())
			value = line.substr(pos+1);
		std::string whitespace("\"= \r\n\t\'");
		pp_s::trim(key, whitespace);
		pp_s::trim(value, whitespace);
		//std::cout << "split '" << line << "' into : '" << key << "' and '" << value << "'" << std::endl;		//##DEBUG
		return true;
	}
	//std::cout << "split '" << line << "' failed" << std::endl;		//##DEBUG
	return false;
}

} //namespace pp_s

