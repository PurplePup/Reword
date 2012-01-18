//PlayMenu.h

#ifndef _PlayMenu_H
#define _PlayMenu_H

#include "play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "roundels.h"
#include "sprite.h"


struct MenuItem
{
	int			_id;		//item id
	SDL_Color	_hoverOff;	//title colour
	SDL_Color	_hoverOn;	//title colour when highlighted
	std::string	_title;		//menu text
	std::string _comment;	//tooltip type help text
	bool		_enabled;	//greyed out?
	bool		_highlight;	//set to highlight even when not hover over (eg diff setting prev selected in diff menu)

	// "Active" box dimensions for touch selection.
	// Should be set by render().
	Rect		_r;     //object rect position/size
	Rect        _rBox;  //box hit test area usually slightly bigger than object

	MenuItem() :
		_id(-1),
		_hoverOff(BLACK_COLOUR),
		_hoverOn(BLACK_COLOUR),
		_enabled(false), _highlight(false) {};
	MenuItem(int id, SDL_Color colour, std::string title, std::string comment, bool enabled=true) :
		_id(id),
		_hoverOff(BLACK_COLOUR), _hoverOn(colour),
		_title(title), _comment(comment),
		_enabled(enabled), _highlight(false) {};

};

typedef std::vector<MenuItem> tMenuItems;

class PlayMenu : public IPlay
{
public:
	PlayMenu(GameData& gd);
	virtual ~PlayMenu() {}

    virtual void init(Input *input);
    virtual void choose(MenuItem i);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual void button(Input* input, ppkey::eButtonType b);
    virtual bool touch(const Point &pt);
    virtual bool tap(const Point &pt);

    void        startMenuMusic();
    void        stopMenuMusic();
	void		setTitle(const std::string &title);
	void		setHelp(const std::string &help, SDL_Color c);
	void        setMenuArea(const Rect &r);
	Uint32		addItem(MenuItem i);	//returns item number
	void		setItem(int item);	//sets the current highlighted item
	MenuItem	getItem(int item);	//gets the ...
	MenuItem	getSelected();
	int			getNextYPos() { return _nextYpos; }

	GameData &	_gd;			//shared data between screens (play classes)

private:
    void        recalcItemPositions();

	Roundels	_title;
	Waiting		_titleW;		//delay between jumbling
	std::string	_help;
	SDL_Color	_helpColor;
	tMenuItems	_itemList;		//list of menu items
	Uint32		_item;			//current item highlighted
	Rect        _menuRect;      //area menu items are placed in
	int			_nextYpos;
	Point       _saveTouchPt;   //save pt at which touch/press occurred
	Waiting     _delayHelp;
};

#endif //_PlayMenu_H
