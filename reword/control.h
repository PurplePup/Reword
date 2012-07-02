#ifndef CONTROL_H
#define CONTROL_H

#include "sprite.h"
#include "i_play.h"
#include <boost/shared_ptr.hpp>

class Control : IPlay
{
public:
    typedef boost::shared_ptr<Sprite> t_pControl;
    //define some control frame types. Specifically CAT_DIS_HIT_IDLE_DOUBLE which allows a frame set
    //with an on/off yes/no type of structure
    enum eCtrlAnimMode { CAM_SIMPLE,                  //simple first to last frame format
                         CAM_DIS_HIT_IDLE_SINGLE,     //[disabled][hit/hilight][...fade x N...][idle]
                         CAM_DIS_HIT_IDLE_DOUBLE,      //[disabled][1.hit/hilight][1...fade x N...][1.idle][2.hit/hilight][2...fade x N...][2.idle]
                         CAM_DIS_HIT_IDLE_TRIPPLE     //[disabled][1.hit/hilight][1...fade x N...][1.idle][2.hit/hilight][2...fade x N...][2.idle][...3rd
                       };

    //Control();
    Control(t_pControl &pCtrl, int id, unsigned int group = 0,
                eCtrlAnimMode mode = CAM_SIMPLE, unsigned int state = 1);
    Sprite * getSprite() { return _pCtrl.get(); }
    bool isPressed() { return _bPressed; }
    void fade(bool bFlip = true);

    void setControlId(int id) {_id = id; }
    void setGroupId(unsigned int group) { _group = group; }

    int getControlId() const { return _id; }
    unsigned int getGroupId() const { return _group; }

    //return true if double anim mode and is frst, else always true as single anim mode
    bool isFirstState() const { return (_animMode == CAM_SIMPLE)?true:(_currState==1); }
    bool isSecondState() const { return (_animMode == CAM_SIMPLE)?false:(_currState==2); }
    bool isThirdState() const { return (_animMode == CAM_SIMPLE)?false:(_currState==3); }
    unsigned int currState() const { return _currState; }
    unsigned int states() const { return (unsigned int)_animMode; } //numeric version of mode

    void setIdleFrame();
    void setActiveFrame();

    //init the level/screen
    virtual void init(Input * /*input*/);
    // drawing operation
    virtual void render(Screen* s);
    // other processing
    virtual void work(Input* input, float speedFactor);
    // notification of button/input state change
    virtual bool button(Input* /*input*/, ppkey::eButtonType /*b*/);

	// screen touch (press)
	virtual bool touch(const Point &pt);
	// screen touch (release)
	virtual bool tap(const Point &pt);

protected:
    t_pControl      _pCtrl;
    int             _id;        //unique id of control (decided when added) & passed to Imageanim
    bool            _bPressed;
    unsigned int    _group;     //bit mask group id (allows finer control)
	Point           _saveTouchPt;   //save pt at which touch/press occurred

    eCtrlAnimMode   _animMode;  //how to automatically animate the control when pressed
//    bool            _animFirst; //only used for CAT_DIS_HIT_IDLE_DOUBLE, true if first else second
    unsigned int    _currState;     //0=disabled, 1=first button, 2=second etc
};

#endif // CONTROL_H
