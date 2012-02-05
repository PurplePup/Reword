//playgamepopup.h

#ifndef _PLAYGAMEPOPUP_H
#define _PLAYGAMEPOPUP_H

#include "play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "playmenu.h"	//for MenuItem struct etc
#include "image.h"
#include <map>
#include <memory>


class PlayGamePopup : public IPlay
{
public:
	PlayGamePopup(GameData& gd, bool bMaxWordFound);
	virtual ~PlayGamePopup() {}

	enum eOptions { POP_CANCEL, POP_SKIP, POP_TOGGLEMUSIC, POP_NEXTTRACK, POP_PREVTRACK, POP_SAVE, POP_QUIT, POP_MAXVAL };
	enum eYNOptions { POP_NO, POP_YES, POP_YN_MAXVAL };	//same as POP_CANCEL etc as same positions in menu
//	void select(Uint32 i) { if (i >= POP_MAXVAL) _menuoption = 0; else _menuoption = i; }
	int selectedId() { return _selectedId; }
	bool isSelected() { return _bSelected; }
	void setHasMaxWord(bool b) { _hasMaxWord = b; }

    virtual void init(Input *input);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual void button(Input* input, ppkey::eButtonType b);
    virtual bool touch(const Point &pt);
    virtual bool tap(const Point &pt);

private:
	void choose();
	void prepareBackground();
	int ItemFromId(int id);

	GameData &	_gd;			//shared data between screens (play classes)
	std::string _helpStr;
	int			_menuoption;	//highlight position
	bool		_bSelected;
	int			_selectedId;
	bool 		_hasMaxWord;			//must have a 6 letter word before some menu items available
	Rect 		_touchArea[POP_MAXVAL];
	Rect		_touchYNArea[POP_YN_MAXVAL];

//	tMenuItems	_itemList;
//	tMenuItems	_itemYNList;

	typedef std::map<int, MenuItem> tMenuMap;
	tMenuMap	_itemList;
	tMenuMap	_itemYNList;
	tMenuMap	*_pItems;
	std::auto_ptr<Image> _menubg;

	bool 		_bDoYesNoMenu;		//user must select yes to exit game
	Waiting		_doubleClick;	    //touch support
};

#endif //_PLAYGAMEPOPUP_H
