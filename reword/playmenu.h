//PlayMenu.h

#if !defined _PlayMenu_H
#define _PlayMenu_H

#include "i_play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "roundels.h"
#include "sprite.h"
#include "fontttf.h"
#include "controls.h"
#include "locator.h"
#include "easing.h"


class MenuItem
{
public:
    enum eItemFont { MENU_FONT_DEFAULT, MENU_FONT_TINY, MENU_FONT_CLEAN, MENU_FONT_SMALL, MENU_FONT_MED, MENU_FONT_BIG  };
    enum eStates { eStateGrey = 1, eStateHoverOff, eStateHoverOn };
    enum eItemType { eTitleGrey = 1, eTitleHoverOff, eTitleHoverOn, eCommentGrey, eCommentHoverOff, eCommentHoverOn };

	MenuItem() :
		_id(-1),
		_hoverOff(BLACK_COLOUR),
		_hoverOn(BLACK_COLOUR),
		_enabled(false), _highlight(false)
	{
	};

	MenuItem(int id, SDL_Color colour,
          const std::string &title, const std::string &comment,
          bool enabled=true, eItemFont font = MENU_FONT_DEFAULT) :
		_id(id),
		_hoverOff(BLACK_COLOUR), _hoverOn(colour),
		_enabled(enabled), _highlight(false)
    {
        setTitle(title, font);
        setComment(comment, MENU_FONT_CLEAN);   //always small/clean
    };

    int id() const { return _id; }
    Image * item(eItemType itemNo) { return _itemCache.get((int)itemNo); }

	bool		_enabled;	//greyed out?
	bool		_highlight;	//set to highlight even when not hover over (eg diff setting prev selected in diff menu)

	// "Active" box dimensions for touch selection.
	// Should be set by render().
	Rect		_r;     //object rect position/size
	Rect        _rBox;  //box hit test area usually slightly bigger than object

protected:

    void setTitle(const std::string &text, eItemFont font );//= MENU_FONT_DEFAULT);
    void setComment(const std::string &comment, eItemFont font);// = MENU_FONT_DEFAULT);

	int			_id;		//item id
	SDL_Color	_hoverOff;	//title colour
	SDL_Color	_hoverOn;	//title colour when highlighted
    FontCache   _itemCache;
};

typedef std::vector<MenuItem> tMenuItems;


class PlayMenu : public IPlay
{
public:
    enum eMenuLayout { LAYOUT_CENTER, LAYOUT_LEFT, LAYOUT_RIGHT };

	PlayMenu(GameData& gd);
	virtual ~PlayMenu() {}

    virtual void init(Input *input);
    virtual void choose(MenuItem i);
    virtual void chooseDone();
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual bool button(Input* input, ppkey::eButtonType b);
    virtual bool touch(const Point &pt);
    virtual bool tap(const Point &pt);
    virtual void handleEvent(SDL_Event &sdlevent);

	void		setTitle(const std::string &title);
	void		setHelp(const std::string &help, SDL_Color c);
	void        setMenuArea(const Rect &r);
	Uint32		addItem(MenuItem i);	//returns item number
	void		setItem(int item);	//sets the current highlighted item
	MenuItem	getItem(int item);	//gets the ...
	Rect        getItemWidest();
	MenuItem	getSelected();
	int			getNextYPos() { return _nextYpos; }
	void        setLayout(eMenuLayout layoutType, int offset);
	void        exitMenu();

protected:
    void        recalcItemPositions();

protected:
	GameData &	_gd;			//shared data between screens (play classes)
	tSharedImage _menubg;
	Sprite      _star;
    Roundels    _beta;
    FontCache   _fontCache;
    int         _fadeX;
    Easing      _fadeEase;

	Roundels	_title;
	Waiting		_titleW;		//delay between jumbling
	tMenuItems	_itemList;		//list of menu items
	Uint32		_item;			//current item highlighted
	Rect        _menuRect;      //area menu items are placed in
	int			_nextYpos;
	Point       _saveTouchPt;   //save pt at which touch/press occurred
	Waiting     _delayHelp;
	eMenuLayout _layoutType;
	int         _layoutOffset;
    Waiting		_doubleClick;	//touch support
    Controls    _controlsMenu;
    bool        _bSetStarPos;
};

#endif //_PlayMenu_H
