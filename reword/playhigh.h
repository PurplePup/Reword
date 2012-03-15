//playhigh.h

#ifndef _PLAYHIGH_H
#define _PLAYHIGH_H

#include "i_play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "states.h"
#include "roundels.h"
#include "image.h"
#include "controls.h"
#include <memory>

class PlayHigh : public IPlay
{
public:
	PlayHigh(GameData& gd);
	virtual ~PlayHigh() {}

    virtual void init(Input *input);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual void button(Input* input, ppkey::eButtonType b);

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
	bool isEditing() { return _pos != -1; }
	void setEditing(bool b) { _pos = b?0:-1; }
    void updateScrollButtons();
    void ControlEvent(int event, int ctrl_id);

private:
	GameData &	_gd;		//shared data between screens (play classes)
	tSharedImage _menubg;

	int			_pos;
	int			_currPos;		//which of the 3 inits you are currently editing
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

	tHiScoreEntry _curr;		//temp inits during payer editing

	int			_diff;			//curr difficulty hiscore table to display
	SDL_Color	_diffColour;	//colour of text to denote hi score difficulty
	int			_mode;			//locally used game mode
	std::string	_description;	//"mode : difficulty"

	Roundels	_title;
	Waiting		_titleW;		//delay between jumbling
	Waiting		_doubleClick;	//touch support

    Controls    _controlsHigh;
};

#endif //_PLAYHIGH_H
