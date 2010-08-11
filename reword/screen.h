//based on Dave Parkers code

/***************************************************************************
 *   Copyright (C) 2006 by Dave Parker                                     *
 *   drparker@freenet.co.uk                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef SCREEN_H
#define SCREEN_H

#include <SDL.h>

#include "error.h"
#include "surface.h"

class Screen :public Error, public Surface
{
public:
    // Construct 16 bit colour screen of given size
    Screen (int w, int h);
    ~Screen();
        
	void lock(void);		// Lock screen
    void unlock(void);		// Unlock screen
    void update(void);		// Update whole screen (flip)

    // Accessor Methods
	bool initDone() const { return _init; }

	//statics
	static int _width;
	static int _height;
	static int width() { return _width; }
	static int height() { return _height; }


private:
    //Screen surface
	bool	_init;

};

#endif //SCREEN_H

