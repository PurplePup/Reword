//playgame.h

#ifndef _PLAYGAME_H
#define _PLAYGAME_H

#include "play.h"		//IPlay interface
#include "screen.h"
#include "input.h"
#include "gamedata.h"	//also holds constants and stuff
#include "waiting.h"
#include "spritemgr.h"
#include "roundels.h"
#include "playgamepopup.h"
#include "controls.h"

#include <string>
#include <stack>
#include <algorithm>
#include <memory>


//a class to wrap the play state (not the game state) so that within game state ST_GAME we
//have the play states (PG_PLAY, PG_WAIT etc) that direct the processing to different render
//and work functions. This allows us to separate things like the popup menu render into its
//own function rather than embedding it in the main play render function with lots of if/thens.
template <class T>
class StateInPlay
{
public:
	StateInPlay() : _init(false) {}		//no default set yet
	StateInPlay(T defaultState) : _defaultState(defaultState), _init(true) {}
	~StateInPlay() {}

	void setDefault(T defaultState) { _defaultState = defaultState; _init = true; }
	long size() { return (long)_state.size(); }
	void clear() { while (!_state.empty()) _state.pop(); }

	T push(T val) {	//and return previous top value
			T prev = val;
			if (_state.empty())
			{
				setDefault(val);	//first push, set as default too
				_state.push(val);
			}
			else
			{
				prev = _state.top();
				if (prev != val)
					_state.push(val);	//only push if prev is different
			}
			return prev;
			}
	T pop() {	//return prev value or default if stack is empty
			if (_state.empty()) return _defaultState;
			_state.pop();
			return _state.top();
			}
	T top() {
			if (_state.empty()) return _defaultState;
			return _state.top();
			}
	bool operator==(const T &t) { return (top() == t); }	//test against state value not state object

protected:
	T _defaultState;
	bool _init;
	std::stack<T> _state;
};


struct DisplayWord //: public DictWord
{
public:
	short	_x;
	short	_y;
	short	_len;
};


class PlayGame : public IPlay
{
public:
	PlayGame(GameData& gd);
	virtual ~PlayGame();

    virtual void init(Input *input);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual void button(Input* input, ppkey::eButtonType b);
    virtual bool touch(const Point &pt);   //press
    virtual bool tap(const Point &pt);     //release

//	static 	void pushSDL_Event(int code, void *data1 = NULL, void *data2 = NULL);

protected:

	//state funcion pointer handling
	void stateFn(eState state);
	void statePush(eState state);
	eState statePop();

	void handleEvent(SDL_Event &sdlevent);
	void exit(eGameState toState);

	void newGame();
	bool newLevel();
	void startCountdown();
	void stopCountdown();
	void clearEventBuffer();
	int maxCountdown();
	void showSuccess(eSuccess newSuccess, int newBonus = 0);
	void startPopup(Input *input);
	void stopPopup();
	void handlePopup();
	int tryWordAgainstDict();
	void tryWord();
    bool foundEnoughWords();
	bool foundAllWords();
	void fillRemainingWords();
	void doMoveOn();
	void doPauseGame();
	void doDictionary();
	void prepareBackground();
    void calcArcadeNeededWords();

	void render_play(Screen*);	//main play render fn
	void render_wait(Screen*);	//waiting for PG_END, do "game over" anim etc
	void render_end(Screen*);	//main play at end
	void render_pause(Screen*);	//pause screen
	void render_popup(Screen*);	//popup menu

    void work_play(Input*, float);
    void work_wait(Input*, float);
    void work_end(Input*, float);
    void work_pause(Input*, float);
    void work_popup(Input*, float);

    void button_play(Input*, ppkey::eButtonType);
    void button_wait(Input*, ppkey::eButtonType);
    void button_end(Input*, ppkey::eButtonType);
    void button_pause(Input*, ppkey::eButtonType);
    void button_popup(Input*, ppkey::eButtonType);

	bool touch_play(const Point &pt);
	bool touch_end(const Point &pt);
	bool touch_default(const Point &pt);
    bool touch_pause(const Point &pt);

	void commandWordToLast();
	void commandClearAllToTop();
	void commandJumbleWord();
	void commandTryWord();

    void slideRoundButtonsIn();
    void slideRoundButtonsOut();

private:
	GameData &	_gd;			//shared data between screens (play classes)
	tSharedImage _gamebg;
	tSharedImage _scorebar;
	ImageAnim	_scratch;
	ImageAnim	_boxes;
	ImageAnim	_cursor;

	//function pointers for render(), work(), button() etc
	void (PlayGame::*pRenderFn)(Screen*);
    void (PlayGame::*pWorkFn)(Input*, float);
	void (PlayGame::*pButtonFn)(Input*, ppkey::eButtonType);
	bool (PlayGame::*pTouchFn)(const Point &pt);

    enum { CTRLGRP_BUTTONS = 1, CTRLGRP_LETTERS=2 };    //...4,8,16 etc
    Controls    _controlsPlay;

	std::string _tmpStr;		//used in render()
	std::string _tmpStr2;		//used in render()
	std::string _tmpDefStr;		//used in render()
	bool        _tmpDefMore;    //..

	int _maxwordlen;			//max length word found so far (per level)
	int _longestWordLen;		//longest word length for this level (6,7 8 etc)
	int _shortestWordLen;		//shortest word length for this level (3,4 5 etc)

	bool _inputL, _inputR;		//if L+R(+CLICK) pressed
	bool _bAbort;				//if user presses L+R+CLICK

	int _xxWordHi, _yyWordHi;	//highlight position of word to display description of

	std::string _dictWord;      //used to pass to dict screen

	//x offsets to draw 3, 4, 5 and 6 (etc) word boxes under main display
	int _boxOffsetY;				//added position below scratch area
	int _boxOffset[TARGET_MAX+1];	//pos across screen for found word boxes
	int _boxLength[TARGET_MAX+1];	//and pixel length of each box displayed (for touch support)
    int _boxWordNeeded[TARGET_MAX+1];   //number of required/needed words in a column (in Arcade mode)
    int _boxWordOffset[TARGET_MAX+1];   //offset into column word list so we can scroll up/down word list

    int _nWordBoxHighlightOffset;   //highlighting a word (for dictionary)
    int _nWordBoxEmptyOffset;       //column with no words
    int _nWordBoxNeededOffset;      //box showing a word should be attempted

	int	_xScratch;		//positions for roundels, change depending on platform
	int _yScratchTop;
	int _yScratchBot;

    //round action buttonpositions next to main letters
   	int _posRButtonLeft, _posRButtonRight, _posRButtonTop, _posRButtonBot;

	int _gamemenu_x, _gamemenu_icon_x, _gamemusic_icon_x, _score0_x, _words0_x, _countdown0_x;
	int _gamemenu_y, _gamemenu_icon_y, _gamemusic_icon_y, _score0_y, _words0_y, _countdown0_y;

	//timers
	SDL_TimerID _countdownID;	//timer used to show timer countdown in game
	static Uint32 next_time;	//used for time_left() fn

	//a list of found words so far (this level)
	typedef std::deque<DictWord> tWordsFoundList;
	tWordsFoundList _wordsFound[TARGET_MAX+1];	//for 3, 4, 5 and 6 letter word
	typedef std::deque<DisplayWord> tWordsFoundPos;
	tWordsFoundPos _wordsFoundPos[TARGET_MAX+1];	//for 3, 4, 5 and 6 letter word

	eState		_state;			//local states for this screen
	StateInPlay<eState> _states;

 // SpriteMgr   _sprites;

	eSuccess	_success;		//used in drawGame()
	int			_bonusScore;
	int 		_fastest;		//quickest time to get a 6 (or set to 0 if not)
	int			_fastestCountStart;	//starting countdown so we can calc fastest times
	Waiting		_waiting;		//for PG_WAIT state, wait before continuing (after level finished)
	Waiting		_doubleClick;
	int			_randomTitle;
	Rect        _pause_rect;

	Roundels	_round;			//for roundel game letter sprites
	tAutoRoundels _roundPaused;	//"PAUSED" in middle of screen
	tAutoRoundels _roundDict;		//"xxxxx" word highlighted for dictionary display

	//different modes during play that are handled by different classes
	PlayGamePopup	*_pPopup;
    IPlay           *_play;

    int _debugTotalLetters, _debugNeededAll, _debugNeededNow;
};

#endif //_PLAYGAME_H
