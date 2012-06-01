#ifndef CONTROLS_H
#define CONTROLS_H

#include "i_play.h"
#include "control.h"
#include <vector>

class Controls : IPlay
{
public:
    typedef std::vector<Control> t_controls;

    Controls();
    int add(Control &ctrl);
    Control * getControl(int id);       //IPlay interface
    Sprite *  getControlSprite(int id); //Sprite data

    bool showControl(bool bShow, int id);
    void showGroup(bool bShow, unsigned int groupMask);
    void showAllControls(bool bShow = true, int exceptID = 0);

    bool enableControl(bool bEnable, int id);

    //init the level/screen
    virtual void init(Input * input);
    // drawing operation
    virtual void render(Screen * s);
    // other processing
    virtual void work(Input * input, float speedFactor);
    // notification of button/input state change
    virtual void button(Input * input, ppkey::eButtonType b);

	// screen touch (press)
	virtual bool touch(const Point &pt)
        { (void)(pt); assert(0); return false; }    //do not confuse touched()
	// screen touch (release)
	virtual bool tap(const Point &pt)
        { (void)(pt); assert(0); return false; }    //do not confuse tapped()

	// screen touch (press) of one of the controls?
	int touched(const Point &pt);
	// screen touch (release) ofone of the controls?
	int tapped(const Point &pt);

protected:

    t_controls  _controls;
};

#endif // CONTROLS_H
