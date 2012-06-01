 //game.h

#ifndef GAME_H
#define GAME_H

#include <memory> //definition of auto_ptr
#include "input.h"
#include "screen.h"
#include "audio.h"
#include "error.h"
#include "i_play.h"	//IPlay interface decl
#include "gamedata.h"
#include "resource.h"

class Game : public Error
{
public:
	Game();
	~Game();

	bool 		init(GameOptions &options);
	bool 		run(void);	//main game loop

protected:
	void		splash();
    bool        loadResources();
	bool		play(IPlay *p);

private:
	bool		_init;

	Screen		*_screen;
	Input		*_input;
	IAudio		*_audio;
	GameData	*_gd;
    ResourceImg	_images;
};

#endif //GAME_H
