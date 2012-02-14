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
                [grey][bright][light][lighter][...][idle](repeat [bright][light][lighter][...][idle])
                for CAM_DIS_HIT_IDLE_DOUBLE anim type.

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

Control::Control(t_pControl &pCtrl, int id, unsigned int group,
                    eCtrlAnimMode type, bool bFirstMode) :
    _pCtrl(pCtrl), _id(id), _bPressed(false), _group(group),
    _animMode(type), _animFirst(bFirstMode)
{
    assert (pCtrl.get() != 0);  //must construct with valid control
    assert (id != 0);           //must give it an id (pref unique)

    pCtrl->setId(id);
    setIdleFrame();
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

void Control::setIdleFrame()
{
    if (_animMode == CAM_DIS_HIT_IDLE_SINGLE //only has first block (after 0 frame 'disabled')
        || _animMode == CAM_SIMPLE) //only has first block (no 0 frame 'disabled')
    {
        _pCtrl->setFrameLast();
    }
    else //if (_animMode == CAM_DIS_HIT_IDLE_DOUBLE)
    {
        if (_animFirst)
            _pCtrl->setFrame((_pCtrl->getFrameCount()-1)/2);    //end of first block
        else
            _pCtrl->setFrame(_pCtrl->getFrameCount()-1);  //end of second block
    }
}

void Control::setActiveFrame()
{
    if (_animMode == CAM_DIS_HIT_IDLE_SINGLE)
    {
        _pCtrl->setFrame(1);    //only has first block (after 0 frame 'disabled')
    }
    else if (_animMode == CAM_DIS_HIT_IDLE_DOUBLE)
    {
        if (_animFirst)
            _pCtrl->setFrame(1);    //first block
        else
            _pCtrl->setFrame(((_pCtrl->getFrameCount()-1)/2)+1);  //second block
    }
    else    //(_animMode == CAM_SIMPLE)
    {
        _pCtrl->setFrame(0);
    }
}

// screen touch (press) - highlight the control
bool Control::touch(const Point &pt)
{
    //if control has a touch ID, then send it if touched
    if (_pCtrl->isVisible() && _pCtrl->isTouchable() && _pCtrl->contains(pt))
    {
        //set 'active' highlight frame
        _bPressed = true;
        setActiveFrame();
		ppg::pushSDL_Event(USER_EV_CONTROL_TOUCH, reinterpret_cast<void *>(_id), 0);
		_saveTouchPt = pt;      //so test if release pos is in same menu item
        return true;
    }
    if (isPressed()) fade(false);
    return false;
}

// screen touch (release) - fade control highlight back to unpressed
bool Control::tap(const Point &pt)
{
    //if control has a tap ID, then send it when tapped
    if (_pCtrl->isVisible() && _pCtrl->isTouchable() && _pCtrl->contains(pt))
    {
        if (_pCtrl->contains(_saveTouchPt))  //same ctrl as when touch occurred
        {
            fade();
            ppg::pushSDL_Event(USER_EV_CONTROL_TAP, reinterpret_cast<void *>(_id), 0);
            return true;
        }
    }
    if (isPressed()) fade(false);
    return false;
}

//fade/unpress the control - up to caller to determine visible etc
//Note: bFlip option only needed for CAM_DIS_HIT_IDLE_DOUBLE control types where
//      state is flipped on/before fade completes.
void Control::fade(bool bFlip)
{
    _bPressed = false;
    Uint32 frameFrom, frameTo;
    if (_animMode == CAM_DIS_HIT_IDLE_SINGLE)
    {
        frameFrom = 1; frameTo = _pCtrl->getFrameCount()-1;
    }
    else if (_animMode == CAM_DIS_HIT_IDLE_DOUBLE)
    {
        if (bFlip) _animFirst = !_animFirst;   //flip it before fade()
        if (_animFirst)
        {
            frameFrom = 1;
            frameTo = (_pCtrl->getFrameCount()/2);  //end of first block
        }
        else
        {
            frameFrom = (_pCtrl->getFrameCount()/2)+1;  //first of second block
            frameTo = _pCtrl->getFrameCount()-1;    //end of second block
        }
    }
    else //(_animMode == CAM_SIMPLE)
    {
        frameFrom = 0; frameTo = _pCtrl->getFrameCount()-1;
    }

    _saveTouchPt = Point(); //blank
    _pCtrl->startAnim(frameFrom, frameTo, ImageAnim::ANI_ONCE, 50, 0, 0, 100); //unpress
}
