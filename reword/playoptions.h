//playoptions.h

#ifndef _PLAYOPTIONS_H
#define _PLAYOPTIONS_H

#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "roundels.h"
#include "controls.h"
#include "playmenu.h"


#include <string>
#include <vector>

class PlayOptions : public PlayMenu
{
public:
	PlayOptions(GameData& gd);
	virtual ~PlayOptions();

    virtual void init(Input *input);
    virtual void choose(MenuItem i);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual void button(Input* input, ppkey::eButtonType b);

	virtual bool touch(const Point &pt);
	virtual bool tap(const Point &pt);

protected:
    void ControlEvent(int event, int control_id);
    void setupWordFile();

private:

//	GameData &	_gd;			//shared data between screens (play classes)
//	Roundels	_title;
//	Waiting		_titleW;		//delay between jumbling
	Waiting		_doubleClick;
	Controls    _controlsOptn;
    int         _yyStart, _yyGap, _xxStartText, _xxStartCtrls;
    std::vector<std::string> _optionsList;

    std::vector<std::string> _wordFileList;
    int         _wordFileIdx;   //selected word file
    int         _yyWordFile;    //y pos to show word file chosen
};

#endif //_PLAYOPTIONS_H
