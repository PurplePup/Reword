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

#include <string>
#include <memory>
#include <stack>
#include <algorithm>


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
	virtual ~PlayGame() { stopCountdown(); delete _pPopup; }

	enum eSuccess { SU_NONE=0, SU_GOT6, SU_BONUS, SU_BADLUCK, SU_SPEED6, SU_TIMETRIAL, SU_GAMEOVER };
	enum eState { PG_PLAY=0, PG_WAIT, PG_END, PG_DICT, PG_PAUSE }; //, PG_POPUP };

    virtual void init(Input *input);
    virtual void render(Screen* s);
    virtual void work(Input* input, float speedFactor);
    virtual void button(Input* input, Input::ButtonType b);
    virtual void touch(Point pt);

//	static 	void pushSDL_Event(int code, void *data1 = NULL, void *data2 = NULL);

protected:
	
	//state funcion pointer handling
	void stateFn(eState state);
	void statePush(eState state);
	eState statePop();

	void handleEvent(SDL_Event &sdlevent);
	void exit(eGameState toState);

	void newGame();
	void newLevel();
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
	bool allWordsFound();
	void fillRemainingWords();
	void doMoveOn();
	void doPauseGame();
	void doDictionary();
	void prepareBackground();
	void scrollDictUp();
	void scrollDictDown();

	void render_play(Screen*);	//main play render fn
	void render_wait(Screen*);	//waiting for PG_END, do "game over" anim etc
	void render_end(Screen*);	//main play at end 
	void render_dict(Screen*);	//dictionary display
	void render_pause(Screen*);	//pause screen
	void render_popup(Screen*);	//popup menu 

    void work_play(Input*, float);
    void work_wait(Input*, float);
    void work_end(Input*, float);
    void work_dict(Input*, float);
    void work_pause(Input*, float);
    void work_popup(Input*, float);

    void button_play(Input*, Input::ButtonType);
    void button_wait(Input*, Input::ButtonType);
    void button_end(Input*, Input::ButtonType);
    void button_dict(Input*, Input::ButtonType);
    void button_pause(Input*, Input::ButtonType);
    void button_popup(Input*, Input::ButtonType);

	void touch_play(Point pt);
	void touch_end(Point pt);
	void touch_default(Point pt);
	void touch_dict(Point pt);
	
	void commandWordToLast();
	void commandClearAllToTop();
	void commandJumbleWord();
	void commandTryWord();

private:
	GameData &	_gd;			//shared data between screens (play classes)
	std::auto_ptr<Image> _gamebg;

	std::string _tmpStr;		//used in render()
	std::string _tmpStr2;		//used in render()
	std::string _tmpDefStr;		//used in render()

	int _maxwordlen;			//max length word found so far (per level)
	int _longestWordLen;		//longest word length for this level (6,7 8 etc)
	int _shortestWordLen;		//shortest word length for this level (3,4 5 etc)
	
	bool _inputL, _inputR;		//if L+R(+CLICK) pressed
	bool _bAbort;				//if user presses L+R+CLICK

	int _xxWordHi, _yyWordHi;	//highlight position of word to display description of
	std::vector<std::string> _dictDef;
	int	_dictLine;				//offset into _dictDef (ie start at _dictDef.begin+_dictLine)
	int _lines;					//number of lines we can display dictionary entry without scrolling

	//x offsets to draw 3, 4, 5 and 6 (etc) word boxes under main display
	int _boxOffsetY;				//added position below scratch area
	int _boxOffset[TARGET_MAX+1];	//pos across screen for found word boxes
	int _boxLength[TARGET_MAX+1];	//and pixel length of each box displayed (for touch support)

	int	_xScratch;		//positions for roundels, change depending on platform
	int _yScratchTop;
	int _yScratchBot;

	int _score0_x, _words0_x, _countdown0_x;
	int _score0_y, _words0_y, _countdown0_y;

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

	eSuccess	_success;		//used in drawGame()
	int			_bonusScore;
	int 		_fastest;		//quickest time to get a 6 (or set to 0 if not)
	int			_fastestCountStart;	//starting countdown so we can calc fastest times
	Waiting		_waiting;		//for PG_WAIT state, wait before continuing (after level finished)
	Waiting		_doubleClick;
	int			_randomTitle;
	
	Roundels	_round;			//for roundel game letter sprites
	std::auto_ptr<Roundels>	_roundPaused;	//"PAUSED" in middle of screen
	std::auto_ptr<Roundels>	_roundDict;		//"xxxxx" word highlighted for dictionary display
	
	//function pointers for render(), work(), button() etc
	void (PlayGame::*pRenderFn)(Screen*);
    void (PlayGame::*pWorkFn)(Input*, float);
	void (PlayGame::*pButtonFn)(Input*, Input::ButtonType);
	void (PlayGame::*pTouchFn)(Point pt);

	//...
	PlayGamePopup	*_pPopup;

};

#endif //_PLAYGAME_H
