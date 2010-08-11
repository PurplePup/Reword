//waiting.h

#ifndef _WAITING_H
#define _WAITING_H

#include "SDL.h"

class Waiting
{
public:
	Waiting();
	Waiting(Uint32 period);
	void start(Uint32 period = 0, Uint32 delay = 0);	//delay=0 to start immediately

	bool done(bool bReset = false);

private:
	Uint32	_period;
	Uint32	_start;
	Uint32	_delay;
};


#endif //_WAITING_H
