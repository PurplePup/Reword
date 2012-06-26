#ifndef EASING_H
#define EASING_H

#include <SDL.h>


struct Ease
{
    Ease() : t(0), b(0), c(0), d(0), s(0), bDone(false) {}
    float t;  //time ms (usually 0)
    float b;  //start pos
    float c;  //total change? ie start pos + dist to end pos
    float d;  //duration ms
    float s;
    bool bDone;
};

class Easing
{
public:
    enum eType { EASE_LINEAR, EASE_OUTBOUNCE, EASE_INOUTSINE, EASE_OUTQUART, EASE_OUTELASTIC };

    Easing();
    void setup(eType ease, float time, float begin, float change, float duration, float s=0.0);
    float work();
    bool done() const { return _ease.bDone; }

    //all easing functions are public to allow calling without helper functions above
    float easeLinear(Ease e);
    float easeOutBounce(Ease e);
    float easeInOutSine(Ease e);
    float easeOutQuart(Ease e);
    float easeOutElastic(Ease e);


private:

    Ease    _ease;
    Uint32  _prevTicks;

    float (Easing::*_pEasingFn)(Ease);
};

#endif // EASING_H
