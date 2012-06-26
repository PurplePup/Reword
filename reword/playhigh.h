//playhigh.h

#ifndef _PLAYHIGH_H
#define _PLAYHIGH_H

#include "i_play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "states.h"
#include "roundels.h"
#include "image.h"
#include "controls.h"
#include "gamedata.h"	//also holds constants and stuff

#include <memory>


class RoundelsKbd : public Roundels
{
public:
    //add split draw, over 3 lines for kbd layout and
    //bottom 3 letters selected should use the new roundels
    //setBottomMax function

    void setKbdLetters(tSharedImage &letters, int x, int y, int gap);

//	void draw(Surface *s);
//	void work();

};


class PlayHigh : public IPlay
{
public:
	PlayHigh(GameData& gd);
	virtual ~PlayHigh() {}

    virtual void init(Input *input);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual void button(Input* input, ppkey::eButtonType b);
    virtual void handleEvent(SDL_Event &sdlevent);

	virtual bool touch(const Point &pt);
	virtual bool tap(const Point &pt);

protected:
	void setDifficulty(eGameDiff diff);
	void setMode(eGameMode mode);
	void setDescription();
	void prepareBackground();
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
	bool isEditing() { return _bEditing; }
	void setEditing(bool b);
	void updateKbdCursor();
    void updateScrollButtons();
    void ControlEvent(int event, int ctrl_id);
    void insertNewScore();

private:
	GameData &	_gd;		//shared data between screens (play classes)
	tSharedImage _menubg;

	int			_pos;           //new position in score table
	bool        _bEditing;      //user entering inits
	bool        _bLastActionTouch;  //last selection was a touch event not D-pad movement etc

//	int			_currPos;		//which of the 3 inits you are currently editing

	int			_yyGap;			//gap between hiscore lines
	int			_xxGap;			//gap between hiscore items on line
	int 		_xxStart;       //offset to start items at
	int			_xDiffLen;		//len of difficulty string
	int			_xCharLen;		//single char length for calc initials input
	int			_xInitsLen;		//len of inits col
	int			_xScoreLen;		//len of score col
	int			_xWordsLen;		// .. words
	int			_xTimesLen;		// .. times
	int         _maxGap;

	HiScoreEntry _curr;		//temp inits during payer editing

	int			_diff;			//curr difficulty hiscore table to display
	SDL_Color	_diffColour;	//colour of text to denote hi score difficulty
	int			_mode;			//locally used game mode
	std::string	_strMode, _strDiff;	//mode & difficulty descriptions for display

	Roundels	_title;

	RoundelsKbd _kbd;           //3 rows displayed as kbd q-p, a-l, z-m
	int         _kbdTileW, _kbdTileH;
	t_pSharedSpr _cursorSpr;     //highlight cursor for non touch movement

	Waiting		_titleW;		//delay between jumbling
	Waiting		_doubleClick;	//touch support

    Controls    _controlsHigh;

};

#endif //_PLAYHIGH_H
