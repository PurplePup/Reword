#ifndef CONTROLS_H
#define CONTROLS_H

#include "play.h"
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
    bool enableControl(bool bEnable, int id);
    void enableAllControls(bool bEnable = true, int exceptID = 0);

    //init the level/screen
    virtual void init(Input * input);
    // drawing operation
    virtual void render(Screen * s);
    // other processing
    virtual void work(Input * input, float speedFactor);
    // notification of button/input state change
    virtual void button(Input * input, IInput::eButtonType b);

	// screen touch (press)
	virtual bool touch(const Point &pt) { assert(0); }   //do not confuse touched()
	// screen touch (release)
	virtual bool tap(const Point &pt) { assert(0); }     //do not confuse tapped()

	// screen touch (press) of one of the controls?
	int touched(const Point &pt);
	// screen touch (release) ofone of the controls?
	int tapped(const Point &pt);

protected:

    t_controls  _controls;
};

#endif // CONTROLS_H
