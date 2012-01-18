//helpers.h

#ifndef _HELPERS_H
#define _HELPERS_H

#include <string>
#include <vector>

namespace pptxt	//pp string functions
{

	//string functions
	void makeUpper(std::string &s);
	void makeLower(std::string &s);
	void trimRight(std::string &s, const std::string &tokens);
	void trimLeft(std::string &s, const std::string &tokens);
	void trim(std::string &source, const std::string &tokens);
	void makeAlpha(std::string &s);
	bool endsWith(const std::string &str, const std::string &match, bool caseSensitive = false);
	void buildTextPage(std::string &inStr, unsigned int nCharsPerLine, std::vector<std::string> &outVect);
	bool splitKeyValuePair(const std::string &line, std::string &key, std::string &value);


}

	//math functions
	//static int round(float fl);

	//rand functions
//	static int RandomInt(unsigned int limit);

#endif //_HELPERS_H
