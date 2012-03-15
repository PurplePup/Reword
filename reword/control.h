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
                         CAM_DIS_HIT_IDLE_DOUBLE      //[disabled][1.hit/hilight][1...fade x N...][1.idle][2.hit/hilight][2...fade x N...][2.idle]
                       };

    //Control();
    Control(t_pControl &pCtrl, int id, unsigned int group = 0,
                eCtrlAnimMode mode = CAM_SIMPLE, bool bFirstMode = true);
    Sprite * getSprite() { return _pCtrl.get(); }
    bool isPressed() { return _bPressed; }
    void fade(bool bFlip = true);

    void setControlId(int id) {_id = id; }
    void setGroupId(unsigned int group) { _group = group; }

    int getControlId() const { return _id; }
    unsigned int getGroupId() const { return _group; }

    //return true if double anim mode and is frst, else always true as single anim mode
    bool isFirstMode() { return (_animMode == CAM_DIS_HIT_IDLE_DOUBLE)?_animFirst:true; }

    void setIdleFrame();
    void setActiveFrame();

    //init the level/screen
    virtual void init(Input * /*input*/);
    // drawing operation
    virtual void render(Screen* s);
    // other processing
    virtual void work(Input* input, float speedFactor);
    // notification of button/input state change
    virtual void button(Input* /*input*/, ppkey::eButtonType /*b*/);

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
    bool            _animFirst; //only used for CAT_DIS_HIT_IDLE_DOUBLE, true if first else second
};

#endif // CONTROL_H
