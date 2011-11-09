#ifndef PLAYGAMEDICT_H
#define PLAYGAMEDICT_H

#include "play.h"
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "controls.h"
#include "roundels.h"

#include <memory>


class PlayGameDict : public IPlay
{
public:
    PlayGameDict(GameData& gd, const std::string &strDictWord);
    virtual ~PlayGameDict();

    virtual void init(Input *input);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual void button(Input* input, IInput::eButtonType b);

	// screen touch (press)
	virtual bool touch(const Point &/*pt*/);
	// screen touch (release)
	virtual bool tap(const Point &/*pt*/);
    // dragging (with button or finger down) for slides/drag/highlight etc
//  virtual bool drag(const Point &/*pt*/) { return false; /*do nothing*/ }

private:

	void scrollDictUp();
	void scrollDictDown();

private:

	GameData &	_gd;			//shared data between screens (play classes)

    Controls    _controlsDict;
	int         _ctrl_id;       //id of pressed control

    std::string _dictWord;
	std::vector<std::string> _dictDef;
	int	_dictLine;				//offset into _dictDef (ie start at _dictDef.begin+_dictLine)
	int _lines;					//number of lines we can display dictionary entry without scrolling

	std::auto_ptr<Roundels>	_roundDict;		//"xxxxx" word highlighted for dictionary display

};

#endif // PLAYGAMEDICT_H
