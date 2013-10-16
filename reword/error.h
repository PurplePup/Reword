//error.h

#if !defined ERROR_H
#define ERROR_H

#include <string>
#include <iostream>

class Error
{
public:
	Error();
	std::string lastError(bool bClear = true);	//return last error set
	bool hasError() { return _lastErr.length(); }

protected:
	void setLastError(std::string, bool bAddSDLError = true);

private:
	std::string _lastErr;
};

#endif
