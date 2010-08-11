//framerate.h

#ifndef _FRAMERATE_H
#define _FRAMERATE_H

#include "SDL.h"

class Framerate
{
public:
	Framerate();

	void	init(float targetFps);
	void	setSpeedFactor();
	void	capFrames();

	//accessors
	inline float speedFactor() const { return _speedFactor; }
	inline Uint32 currTicks() const { return _currentTicks; }
	inline int fps() const { return (int)fps_current; }

private:
	float		_targetFps;
	float		_fps;
	Uint32		_ticksPerSecond;
	Uint32		_currentTicks;
	Uint32		_frameDelay;
	float		_speedFactor;	

	Uint32		fps_last;
	Uint32		fps_count;
	Uint32		fps_current;

	Uint32		_ticks;	//temp var

};

#endif //_FRAMERATE_H

