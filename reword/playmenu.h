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
	// "Active" dimensions for touch selection.
	// Should be set by render().
	Rect		_r;
	MenuItem() :
		_id(-1),
		_hoverOff(BLACK_COLOUR),
		_hoverOn(BLACK_COLOUR),
		_title(""), _comment(""),
		_enabled(false), _highlight(false),
		_r(0, 0, 0, 0) {};
	MenuItem(int id, SDL_Color colour, std::string title, std::string comment, bool enabled=true) :
		_id(id),
		_hoverOff(BLACK_COLOUR), _hoverOn(colour),
		_title(title), _comment(comment),
		_enabled(enabled), _highlight(false),
		_r(0, 0, 0, 0) {};

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
    virtual void button(Input* input, Input::ButtonType b);
    virtual void touch(Point pt);

	void		setTitle(std::string title);
	void		setHelp(std::string help, SDL_Color c);
	Uint32		addItem(MenuItem i);	//returns item number
	void		setItem(int item);	//sets the current highlighted item
	MenuItem	getItem(int item);	//gets the ...
	MenuItem	getSelected();
	int			getNextYPos() { return _nextYpos; }

	GameData &	_gd;			//shared data between screens (play classes)
	
private:

	Roundels	_title;
	Waiting		_titleW;		//delay between jumbling
	std::string	_help;
	SDL_Color	_helpColor;
	tMenuItems	_itemList;		//list of menu items
	Uint32		_item;			//current item highlighted
	Waiting		_doubleClick;	//touch support
	int			_nextYpos;
};

#endif //_PlayMenu_H
