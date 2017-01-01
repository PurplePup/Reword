#if !defined PLAYGAMEDICT_H
#define PLAYGAMEDICT_H

#include "i_play.h"
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "controls.h"
#include "roundels.h"
#include "fontttf.h"
//#include "utils.h"

//#include <memory>


class PlayGameDict : public IPlay
{
public:
    PlayGameDict(GameData& gd, const std::string &strDictWord);
    virtual ~PlayGameDict();

    virtual void init(Input *input, Screen * scr);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual bool button(Input* input, ppkey::eButtonType b);

	// screen touch (press)
	virtual bool touch(const Point &/*pt*/);
	// screen touch (release)
	virtual bool tap(const Point &/*pt*/);
    // dragging (with button or finger down) for slides/drag/highlight etc
//  virtual bool drag(const Point &/*pt*/) { return false; /*do nothing*/ }

private:

	void scrollDictUp();
	void scrollDictDown();
    void updateScrollButtons();
    void ControlEvent(int event, int ctrl_id);

private:

	GameData &	            _gd;			//shared data between screens (play classes)
    Controls                _controlsDict;
	tSharedImage            _menubg;

	Waiting		            _doubleClick;
    std::string             _dictWord;
	std::vector<std::string> _dictDef;
//	tImageMap               _imageMap;
	int	_dictLine;			//offset into _dictDef (ie start at _dictDef.begin+_dictLine)
	int _lines;				//number of lines we can display dictionary entry without scrolling

	tAutoRoundels           _roundDict;		//"xxxxx" word highlighted for dictionary display
    std::string             _helpMsg;

//    FontCache               _fontCache;
//    int                     _indexHelpMsg;
};

#endif // PLAYGAMEDICT_H
