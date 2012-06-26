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
#include "spritemgr.h"

//loadable game options and set using the options screen
struct GameOptions
{
    GameOptions();
    ~GameOptions();
    bool load();
    bool save();

    void setSingleTap(bool b);
    void setDefaultSfxOn(bool b);
    void setDefaultMusicOn(bool b);
    void setDefaultDiff(eGameDiff e);
    bool setDefaultWordFile(const std::string &wordFile);

    //command line options, override default options
    bool        _bSound;    //if false, loads null IAudio
    bool        _bMute;     //true if mute fx and music at startup (cmd line option)

    //default options, set in the options screen
	bool        _bSingleTapMenus;
	bool        _bDefaultSfxOn;
	bool        _bDefaultMusicOn;
	std::string _defaultWordFile;   //"words/reword_english_uk.txt"
	std::string _defaultMusicDir;   //"music"
	eGameDiff   _defaultDifficulty; //0,1,or 2 (easy, med or hard)
private:
    bool        _bDirty;
    std::string _optionsFile;
};

//to persist global data throughout app
class GameData
{
public:
	GameData();
	GameData(GameOptions &opt);
	~GameData();

    void setOptions(const GameOptions &opt) { _options = opt; }
	void init();

	bool isLoaded() {return _init;}
	void setDiffLevel(eGameDiff newDiff);

	void saveQuickState();
	bool loadQuickState();

	//Resources - not in resource handler
	/////////////////////////////////////

	//fonts
	FontTTF _fntTiny;
	FontTTF _fntSmall;
	FontTTF _fntMed;
	FontTTF _fntBig;
	FontTTF _fntClean;
	FontTTF _fontTiny;

	//sound effectes etc
	Mix_Chunk *_fxCountdown;	//countdown ping noise
	Mix_Chunk *_fxBadword;		//word not in dictionary noise
	Mix_Chunk *_fxOldword;		//word already found and now shown in list
	Mix_Chunk *_fx6notfound;	//6 letter word not found after timeout
	Mix_Chunk *_fx6found;		//6 letter word found
	Mix_Chunk *_fxFound;		//non 6 letter word found
	Mix_Chunk *_fxBonus;		//all words found in time
	Mix_Chunk *_fxWoosh;		//jumble letters sound
	Mix_Chunk *_fxRoundel;		//roundel press
	Mix_Chunk *_fxControl;		//control press

	Mix_Music *_musicMenu;		//menu music, fades in game

	//Game vars
	///////////////////////////////

    GameOptions _options;

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
    std::string _prev_inits;        //for scoreboard

    SpriteMgr   _effects;       //sprite effects used in main game loop

private:
	bool		_init;

};

#endif //GAMEDATA_H
