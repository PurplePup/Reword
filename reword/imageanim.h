//imageanim.h

#ifndef _IMAGEANIM_H
#define _IMAGEANIM_H

#include "image.h"
#include "waiting.h"
#include "signal.h"

#include <string>
#include <vector>

class ImageAnim //: public Image
{
public:
	enum eAnim {	ANI_NONE,		//static, not moving
					ANI_ONCE,		//iterate then stop
					ANI_HIDE,		//hide/stop when end point reached
					ANI_LOOP,		//loop forever, restarting frame 1 when last frame done
					ANI_REVERSE,	//reverse movement when end/start point reached
					ANI_CUSTOM      //set a custom frame sequence
				};
    enum eAnimDir { DIR_FORWARD, DIR_BACKWARD };

	ImageAnim();
	ImageAnim(std::string fileName, bool bAlpha, Uint32 nFrames);
//	ImageAnim(const Image &img);
    ImageAnim(tSharedImage &img);
	virtual ~ImageAnim() {}
//	virtual bool load(std::string fileName, int iAlpha = -1, Uint32 nFrames = 0);	//default no alpha, 1 frames

	//position
	virtual void setPos(float x, float y)	{ _x = x; _y = y; }
	float getXPos() const			{ return _x; }
	float getYPos()	const			{ return _y; }

	void setBounds(int inflateBy);
    Rect bounds() const;

    //boost signal/slot events
    //typedef boost::signal<void (int, int)> EventSignal;
    typedef Gallant::Signal2<int, int> EventSignal;
    EventSignal _sigEvent;

	//animation
	void setFrame(Uint32 frame);
	Uint32 getFrame() const			{ return _frame; }

	void setFrameCount(Uint32 nFrames);
	Uint32 getFrameCount() const		{ return _nFrames; }

	void startAnim(int firstFrame_0, int lastFrame_N, eAnim animType,
                Uint32 rate_ms, Uint32 repeat_N = 0, Uint32 restartIn_ms = 0, Uint32 delayStart_ms = 0);
	void setFrameRange(Uint32 firstFrame, Uint32 lastFrame);
	void setFrameLast();
	void setRepeat(Uint32 r)		{ _repeat = r; }

    void clearAnimCustom();
    bool addAnimCustom(Uint32 frame);
    bool addAnimCustom(Uint32 frameFrom, Uint32 frameTo);   //from->to inclusinve

    void setObjectId(int objectId) { _objectId = objectId; }
    int getObjectId() { return _objectId; }

    void setAnimDelay(Uint32 delay, bool bRestart = false) { _delayA.start(delay); _bDelayRestart = bRestart; }   //before anim starts & each repeat?
	void setAnimRate(Uint32 rate)	            { _waitA.start(_rateA = rate, 0); }
	void setAnimType(eAnim anim);
	void setAnimDir(eAnimDir dir)		        { _frameDir = (dir == DIR_BACKWARD)?-1:1; }	//anim start dir (frame +/-) +1 for forward, -1 for back
	void setAnimRestart(Uint32 r, Uint32 d=0)	{ _restartA.start(_restart = r, d); }
	void setVisible(bool b)			            { _visible = b; }
	void pauseAnim(bool b = true)	            {
	     _pauseA = b;
	     }			//stop/start animating
	void toggleAnim()				            { _pauseA = !_pauseA; }		//toggle pause state
	void resetAnim()				            { _frame = _firstFrame; }	//start anim sequence at start
	inline bool canAnim() const		            {
	     return
	     (_nFrames>1);
	     }// && !_pauseA); }		//has more than 1 frame & not paused
	inline bool isVisible() const 	            { return _visible; }

    //tile functions
	int tileW() const       { return _image->tileW(); }
	int tileH() const       { return _image->tileH(); }
	int tileCount()	const   { return _image->tileCount(); }
	SDL_Rect tileRect(int i) { return _image->tileRect(i); }
//	Image * createImageFromThis(int tileNum, int iAlpha = -1);	//return a new image from a tile in this image
//	void createThisFromImage(Image &image, int tileNum = -1, int iAlpha = -1);	//create this image using an existing image (tile)

	////static functions available to all
	//static SDL_Rect tileRect(int i, int w, int h);	//[static] just return the Rect for requested tile

	void setImage(tSharedImage &img);
	tSharedImage &getImage() { return _image; }

	//specific blit for Image class types
	SDL_Surface *surface(void) const { return _image->surface(); }
	void blitTo(Surface* dest, int destX = 0, int destY = 0, int tileNum = -1);
	void blitFrom(Image* source, int destX = 0, int destY = 0);
	void blitFrom(ImageAnim* source, int tileNum = -1, int destX = 0, int destY = 0);

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
	void workCUSTOM(void);

protected:

    tSharedImage _image;    //only a shared ptr to existing resource image (so multiple valid)

	//position
	float	_x, _y;		//current screen offset/position

	int     _objectId;  //id passed down from image/sprite/ctrl to pass back in signal

    //tile info
	int     _tileW, _tileH; //actual individual tile w & h withing image
	int     _tileCount;	    //number of tiles in image (if setTileSize() used)
    int     _tileWOffset, _tileHOffset;     //value or 0, depend on eTileDir to multiply by

	//animation
	Uint32	_frame;		//current frame
	Uint32	_firstFrame;//frame to reset back to on restart sequence (may not be 0 frame)
	Uint32	_lastFrame;	//last frame in anim loop etc (may not be max frame)
	Uint32	_nFrames;	//number of frames in the sequence
	int		_frameDir;	//+1 or -1
	eAnim	_animType;
	Uint32	_repeat;	//number of times the LOOP or REVERSE is performed
	Uint32	_restart;	//restart animation again in a number of ms after start (useful for 'pinging' an effect)

	int     _inflateBy;	//to increase image bounding rect by

private:
	bool	_visible;	//render or not?
	bool	_pauseA;	//true if not animating (default=false)
	Uint32	_rateA;		//rate of frame change (0 = every update, else ticks between)
	Waiting _waitA;		//animation frame delay
	Waiting _restartA;	//animation restart period

	Waiting _delayA;	//animation delay before start
	bool    _bDelayRestart; //if the delay is reset for each animation loop

	std::vector<Uint32> _animCustom;    //custom frame iteration
	Uint32              _frameCustom;   //custom count

};

#endif //_IMAGEANIM_H

