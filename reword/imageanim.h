//imageanim.h

#ifndef _IMAGEANIM_H
#define _IMAGEANIM_H

#include "image.h"
#include "waiting.h"

#include <string>

class ImageAnim : public Image
{
public:
	enum eAnim {	ANI_NONE = 0,		//static, not moving
					ANI_ONCE = 1,		//iterate then stop
					ANI_HIDE = 2,		//hide/stop when end point reached
					ANI_LOOP = 4,		//loop forever, restarting frame 1 when last frame done
					ANI_REVERSE = 8		//reverse movement when end/start point reached
				};

	ImageAnim();
	ImageAnim(std::string fileName, bool bAlpha, Uint32 nFrames);
	ImageAnim(const Image &img);
	virtual ~ImageAnim() {}
	virtual bool load(std::string fileName, int iAlpha = -1, Uint32 nFrames = 0);	//default no alpha, 1 frames

	//position
	virtual void setPos(float x, float y)	{ _x = x; _y = y; }
	float getXPos()					{ return _x; }
	float getYPos()					{ return _y; }

	//animation
	void setMaxFrame(Uint32 nFrames);
	void startAnim(int firstFrame_0, int lastFrame_N, eAnim animType,
                Uint32 rate_ms, Uint32 repeat_N = 0, Uint32 restartIn_ms = 0, Uint32 delayStart_ms = 0);
	void setFrameRange(Uint32 firstFrame, Uint32 lastFrame);
	void setFrame(Uint32 frame);
	Uint32 getFrame() const			{ return _frame; }
	Uint32 getMaxFrame() const		{ return _nFrames; }
	void setRepeat(Uint32 r)		{ _repeat = r; }

    void setAnimDelay(Uint32 delay, bool bRestart = false) { _delayA.start(delay); _bDelayRestart = bRestart; }   //before anim starts & each repeat?
	void setAnimRate(Uint32 rate)	            { _waitA.start(_rateA = rate, 0); }
	void setAnimType(eAnim anim);
	void setAnimDir(int dir)		            { _frameDir = (dir<0)?-1:1; }	//anim start dir (frame +/-) +1 for forward, -1 for back
	void setAnimRestart(Uint32 r, Uint32 d=0)	{ _restartA.start(_restart = r, d); }
	void setVisible(bool b)			            { _visible = b; }
	void pauseAnim(bool b = true)	            { _pauseA = b; }			//stop/start animating
	void toggleAnim()				            { _pauseA = !_pauseA; }		//toggle pause state
	void resetAnim()				            { _frame = _firstFrame; }	//start anim sequence at start
	inline bool canAnim() const		            { return (_nFrames>1); }// && !_pauseA); }		//has more than 1 frame & not paused
	inline bool isVisible() const 	            { return _visible; }

	//main functions
	virtual void work();	//calc new pos/frame etc
	void draw(Surface *s);	//render the image

private:
	//animating functions
	void (ImageAnim::*pWorkFn)(void);

	void workNONE(void);
	void workONCE(void);
	void workHIDE(void);
	void workLOOP(void);
	void workREVERSE(void);

protected:

	//position
	float	_x, _y;		//current screen offset/position

	//animation
	Uint32	_frame;		//current frame
	Uint32	_firstFrame;//frame to reset back to on restart sequence (may not be 0 frame)
	Uint32	_lastFrame;	//last frame in anim loop etc (may not be max frame)
	Uint32	_nFrames;	//number of frames in the sequence
	int		_frameDir;	//+1 or -1
	eAnim	_animType;
	Uint32	_repeat;	//number of times the LOOP or REVERSE is performed
	Uint32	_restart;	//restart animation again in a number of ms after start (useful for 'pinging' an effect)

private:
	bool	_visible;	//render or not?
	bool	_pauseA;	//true if not animating (default=false)
	Uint32	_rateA;		//rate of frame change (0 = every update, else ticks between)
	Waiting _waitA;		//animation frame delay
	Waiting _restartA;	//animation restart period

	Waiting _delayA;	//animation delay before start
	bool    _bDelayRestart; //if the delay is reset for each animation loop
};

#endif //_IMAGEANIM_H

