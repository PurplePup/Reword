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
    Control(t_pControl &pCtrl, int id, unsigned int group = 0);
    Sprite * getSprite() { return _pCtrl.get(); }
    bool isPressed() { return _bPressed; }
    void fade();

    void setID(int id) {_id = id; }
    //void setTouchID(int id) {_touchID = id; }
    //void setTapID(int id) {_tapID = id; }
    void setGroup(unsigned int group) { _group = group; }

    int getID() const { return _id; }
    //int getTouchID() const { return _touchID; }
    //int getTapID() const { return _tapID; }
    unsigned int getGroup() const { return _group; }

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
    t_pControl  _pCtrl;
    int         _id;         //unique id of control (decided when added)
    int         _touchID;    //id of message sent when control touched (pressed)
    int         _tapID;      //id of message sent when control tapped (released)
    bool        _bPressed;
    unsigned int _group;    //bit mask group id (allows finer control)
};

#endif // CONTROL_H
