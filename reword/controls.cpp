////////////////////////////////////////////////////////////////////
/*

File:			controls.cpp

Class impl:		Controls

Description:	A container class to manage a list of (scoped/self deleting)
                controls and to call the control base functions as required.

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			23 August 2011

History:		Version	Date		Change
				-------	----------	--------------------------------


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

#include "controls.h"

Controls::Controls()
{
    //ctor
}

int Controls::add(Control &ctrl)
{
    _controls.push_back(ctrl);
    return (int)_controls.size();
}

Control * Controls::getControl(int id)
{
    //find the control matchng the id and return a ref to it
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        if (it->getID() == id)
            return &(*it);
    }
    return 0; //nullptr
}

Sprite * Controls::getControlSprite(int id)
{
    Control *pc = getControl(id);
    if (pc) return pc->getSprite();
    return 0; //nullptr
}

//init the level/screen
void Controls::init(Input * /*input*/)
{
};
// drawing operation
void Controls::render(Screen* s)
{
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        it->render(s);
    }
};
// other processing
void Controls::work(Input* input, float speedFactor)
{
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        it->work(input, speedFactor);
    }
};
// notification of button/input state change
void Controls::button(Input* /*input*/, IInput::eButtonType /*b*/)
{
};

// screen touch (press) of one of the controls?
int Controls::touched(const Point &pt)
{
    int id(0);
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        if (it->touch(pt))  //changes matched control to 'selected' frame
        {
            //return the id of the control pressed
            id = it->getID();
        }
        else if (it->isPressed()) it->fade();
    }
    return id;
}

// screen touch (release) of one of the controls
int Controls::tapped(const Point &pt)
{
    int id(0);
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        if (it->tap(pt))
        {
            //return the id of the control
            id = it->getID();
        }
        else if (it->isPressed()) it->fade();
    }
    return id;
}

//helper fn to enable or disable a control
bool Controls::enableControl(bool bEnable, int id)
{
    Sprite *ps = getControlSprite(id);
    if (ps)
    {
        ps->setVisible(bEnable);
        return true;
    }
    return false;
}

//enable or disable all controls in one go, with possible exception of a single control
//so enable all but hide one, or disable all but show one.
void Controls::enableAllControls(bool bShow /*=true*/, int exceptID /*=0*/)
{
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        Sprite *ps = it->getSprite();
        if (!ps) continue;
        if (exceptID && it->getID() == exceptID)
        {
            ps->setVisible(!bShow);
        }
        else
        {
            ps->setVisible(bShow);
        }
    }
}