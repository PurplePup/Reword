 //game.h

#ifndef GAME_H
#define GAME_H

#include <memory> //definition of auto_ptr
#include "input.h"
#include "screen.h"
#include "audio.h"
#include "error.h"
#include "play.h"	//IPlay interface decl
#include "gamedata.h"

struct GameOptions
{
    GameOptions() : _bSfx(true), _bMusic(true) {}
    bool _bSfx;
    bool _bMusic;
};

class Game : public Error
{
public:
	Game(const GameOptions &options);
	~Game();

	bool 		init();
	bool 		run(void);	//main game loop

protected:
	void		splash();
	bool		play(IPlay *p);

private:
	bool		_init;

	Screen		*_screen;
	Input		*_input;
//	Audio		*_audio;
	GameData	*_gd;

	GameOptions _options;
};

#endif //GAME_H
