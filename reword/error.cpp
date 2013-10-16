////////////////////////////////////////////////////////////////////
/*

File:			error.cpp

Class impl:		Error

Description:	A simple error class to wrap return messages

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

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


#include "global.h"
#include <SDL.h>
#include <SDL_error.h>
#include "error.h"

using namespace std;


Error::Error() : _lastErr("")
{

}

//set the last error given so calling code can get it
void Error::setLastError(std::string err, bool bAddSDLError /* = true */)
{
	_lastErr = err;
	if (bAddSDLError && _lastErr.length() > 0)
	{
		_lastErr += " - SDL Error: ";
		_lastErr += SDL_GetError();
		std::cerr << _lastErr << std::endl;
	}
}

std::string Error::lastError(bool bClear)
{
    if (bClear && hasError())
    {
        std::string err(_lastErr);
        _lastErr.clear();
        SDL_ClearError();
        return err;
    }
    else return _lastErr;
}

