#ifndef CONTROL_H
#define CONTROL_H

#include "sprite.h"
#include "play.h"
#include <boost/shared_ptr.hpp>

class Control : IPlay
{
public:
    typedef boost::shared_ptr<Sprite> t_pControl;

    //Control();
    Control(t_pControl &pCtrl, int id);
    Sprite * getSprite() { return _pCtrl.get(); }
    bool isPressed() { return _bPressed; }
    void fade();

    void setID(int id) {_id = id; }
    void setTouchID(int id) {_touchID = id; }
    void setTapID(int id) {_tapID = id; }

    int getID() const { return _id; }
    int getTouchID() const { return _touchID; }
    int getTapID() const { return _tapID; }

    //init the level/screen
    virtual void init(Input * /*input*/);
    // drawing operation
    virtual void render(Screen* s);
    // other processing
    virtual void work(Input* input, float speedFactor);
    // notification of button/input state change
    virtual void button(Input* /*input*/, IInput::eButtonType /*b*/);

	// screen touch (press)
	virtual bool touch(const Point &pt);
	// screen touch (release)
	virtual bool tap(const Point &pt);

protected:
    t_pControl  _pCtrl;
    int         _id;         //unique id of control (decided when added)
    int         _touchID;    //id of message sent when control touched (pressed)
    int         _tapID;      //id of message sent when control tapped (released)
    bool        _bPressed;
};

#endif // CONTROL_H
