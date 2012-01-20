////////////////////////////////////////////////////////////////////
/*

File:			control.cpp

Class impl:		Control

Description:	A class to manage a single on screen control (button)
                Each control has a message id to send when it is actioned,
                and an id that uniquely identifies the control.
                This is specific to the style of buttons I'm using which are
                animated using several frames, with the first frame being the
                disabled one and the first being the 'selected' or highlighted
                one. Then each next frame reverts back to the default idle frame:
                [grey][bright][light][lighter][...][idle]

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

#include "control.h"

#include "global.h"
#include "platform.h"

Control::Control(t_pControl &pCtrl, int id, unsigned int group) :
    _pCtrl(pCtrl), _id(id), _touchID(0), _tapID(0), _bPressed(false), _group(group)
{
    assert (pCtrl.get() != 0);  //must construct with valid control
    assert (id != 0);           //must give it an id (pref unique)
}

// init the control
void Control::init(Input * /*input*/)
{
};

// drawing operation
void Control::render(Screen* s)
{
    _pCtrl->draw(s);
};

// other processing
void Control::work(Input* input, float speedFactor)
{
    (void)(input);
    (void)(speedFactor);
    _pCtrl->work();
};

// notification of button/input state change
void Control::button(Input* /*input*/, ppkey::eButtonType /*b*/)
{
};

// screen touch (press) - highlight the control
bool Control::touch(const Point &pt)
{
    //if control has a touch ID, then send it if touched
    if (_pCtrl->isVisible() && _pCtrl->isTouchable() && _pCtrl->contains(pt))
    {
        //set 'active' highlight frame
        _pCtrl->setFrame(1);
        _bPressed = true;
		ppg::pushSDL_Event(USER_EV_CONTROL_TOUCH, reinterpret_cast<void *>(_id), 0);
        return true;
    }
    return false;
}

// screen touch (release) - fade control highlight back to unpressed
bool Control::tap(const Point &pt)
{
    //if control has a tap ID, then send it when tapped
    if (_pCtrl->isVisible() && _pCtrl->isTouchable() && _pCtrl->contains(pt))
    {
        fade();
		ppg::pushSDL_Event(USER_EV_CONTROL_TAP, reinterpret_cast<void *>(_id), 0);
        return true;
    }
    return false;
}

//fade/unpress the control - up to caller to determine visible etc
void Control::fade()
{
    _bPressed = false;
    _pCtrl->startAnim(1, _pCtrl->getMaxFrame(), ImageAnim::ANI_ONCE, 50, 0, 0, 100); //unpress
}
