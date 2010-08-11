//error.h

#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <iostream>

class Error
{
public:
	Error();
	std::string lastError();	//return last error set

protected:
	void setLastError(std::string, bool bAddSDLError = true);

private:
	std::string _lastErr;
};

#endif
