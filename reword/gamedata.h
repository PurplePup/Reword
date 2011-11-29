//gamedata.h
//
//All the types, constants and data used globally for the game;
//fonts, images, scores, counters etc


#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "SDL.h"
#include "SDL_mixer.h"

#include "image.h"
#include "fontttf.h"
#include "states.h"
#include "words.h"		//SDL.h should be declared before this
#include "score.h"
#include "sprite.h"


//to persist global data throughout app
class GameData
{

public:
	GameData();
	~GameData();
	bool isLoaded() {return _init;}
	void setDiffLevel(eGameDiff newDiff);

	//void loadOptions();
	//void saveOptions();

	void saveQuickState();
	bool loadQuickState();

	//Resources
	//////////////////////////////

	//fonts
	FontTTF _fntTiny;
	FontTTF _fntSmall;
	FontTTF _fntMed;
	FontTTF _fntBig;
	FontTTF _fntClean;
	FontTTF _fontTiny;

	//backgrounds & images
	Image _menubg;
	Image _menubg_plain;
	Image _menu_arcade;
	Image _menu_reword;
	Image _menu_speed6;
	Image _menu_timetrial;
	Image _scorebar;
	Image _game_arcade;
	Image _game_reword;
	Image _game_speed6;
	Image _game_timetrial;
	Image _cursor;
	Image _letters;
	Image _boxes;
	Image _gamemenu;    //in-game popup menu

	ImageAnim _scratch; //multiple frames (but not animated)

	Sprite _arrowUp;
	Sprite _arrowDown;
	Sprite _arrowLeft;
	Sprite _arrowRight;
	Sprite _star;
	Sprite _gamemusic_icon;        //2 frames, not animated

	Sprite _word_last_pulse;
	Sprite _word_totop_pulse;
	Sprite _word_shuffle_pulse;
	Sprite _word_try_pulse;

	//sound effectes etc
	Mix_Chunk *_fxCountdown;	//countdown ping noise
	Mix_Chunk *_fxBadword;		//word not in dictionary noise
	Mix_Chunk *_fxOldword;		//word already found and now shown in list
	Mix_Chunk *_fx6notfound;	//6 letter word not found after timeout
	Mix_Chunk *_fx6found;		//6 letter word found
	Mix_Chunk *_fxFound;		//non 6 letter word found
	Mix_Chunk *_fxBonus;		//all words found in time
	Mix_Chunk *_fxWoosh;		//jumble letters sound

	Mix_Music *_musicMenu;		//menu music, fades in game

	//Game vars
	///////////////////////////////

	int			_mainmenuoption;	// (0=play, 1=level, 2=hiscore etc)

	eGameDiff	_diffLevel;
	std::string	_diffName;
	SDL_Color	_diffColour;

	eGameState	_state;		//state of play...
	eGameMode	_mode;		//type of game

	Score		_score;
	Words		_words;		//The one instance of the main word manipulation class

	float		_fact;		//frame rate speed factor

	tWordSet 	_unmatchedWords;	//set of words tested but not matching any in dict

	int 		_current_h;			//used to store screen height as SDL_VideoInfo current_w not available in my build
	int			_current_w;			//used to store screen width

	bool 		_bTouch;			//true if touchscreen/mouse detected

private:
	bool		_init;

};

#endif //GAMEDATA_H
