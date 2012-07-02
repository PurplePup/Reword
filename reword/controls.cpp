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
    //find the control matchng the id and return a ptr to it
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        if (it->getControlId() == id)
            return &(*it);
    }
	std::cerr << "Controls::getControl(" << id << ") failed to find " << std::endl;
    assert(0);
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
bool Controls::button(Input* /*input*/, ppkey::eButtonType /*b*/)
{
    return false;
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
            //save id of the control pressed to pass back
            id = it->getControlId();
            //loop to end - so each control gets a touch() call
        }
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
            //save id of the control pressed to pass back
            id = it->getControlId();
            //loop to end - so each control gets a tap() call
        }
    }
    return id;
}

//helper fn to show or hide a control
bool Controls::showControl(bool bShow, int id)
{
    Sprite *ps = getControlSprite(id);
    if (ps)
    {
        ps->setVisible(bShow);
        return true;
    }
    return false;   //id not found
}

//helper fn to show or hide a defined group of controls
void Controls::showGroup(bool bShow, unsigned int groupMask)
{
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        if (groupMask & it->getGroupId())
        {
            Sprite *ps = it->getSprite();
            if (ps)
            {
                ps->setVisible(bShow);
            }
        }
    }
}

//show or hide all controls in one go, with possible exception of a single control
//so enable all but hide one, or disable all but show one. Useful for [Next] or [Exit]
//only situations.
void Controls::showAllControls(bool bShow /*=true*/, int exceptID /*=0*/)
{
    t_controls::iterator it = _controls.begin();
    t_controls::iterator end = _controls.end();
    for ( ; it != end; ++it)
    {
        Sprite *ps = it->getSprite();
        if (!ps) continue;
        if (exceptID && it->getControlId() == exceptID)
        {
            ps->setVisible(!bShow);
        }
        else
        {
            ps->setVisible(bShow);
        }
    }
}

//show 'disabled' frame (0) or default active frame (last)
bool Controls::enableControl(bool bEnable, int id)
{
    Control *pc = getControl(id);
    if (pc)
    {
        Sprite *ps = pc->getSprite();
        if (!ps) return false;  //fail!
        if (bEnable)
        {
            ps->setTouchable(true);
            pc->setIdleFrame();
            ps->setVisible(true);   //must be visible if enabled
        }
        else
        {
            ps->setTouchable(false);
            ps->setFrame(0);
        }
        return true;
    }
    return false;   //id not found
}

