//global.h

#ifndef _GAME_GLOBAL_H_
#define _GAME_GLOBAL_H_

#include "platform.h"
#include "SDL.h"

const SDL_Color NORMAL_COLOUR	= {0xDD,0xDD,0xDD,0};
const SDL_Color BLACK_COLOUR	= {0x00,0x00,0x00,0};
const SDL_Color WHITE_COLOUR	= {0xFF,0xFF,0xFF,0};
const SDL_Color GREY_COLOUR		= {0xAA,0xAA,0xAA,0};
const SDL_Color RED_COLOUR		= {0xFF,0x00,0x00,0};
const SDL_Color GREEN_COLOUR	= {0x22,0xAA,0x22,0};
const SDL_Color BLUE_COLOUR		= {0x40,0x40,0xB0,0};
const SDL_Color YELLOW_COLOUR	= {0xFF,0xFF,0x50,0};
const SDL_Color ORANGE_COLOUR	= {0xF0,0x84,0x00,0};
//const SDL_Color BG_COLOUR		= {0xdb,0xdb,0xb7,0};	//same as loaded bg png's
const SDL_Color GOLD_COLOUR		= {0xba,0xa4,0x05,0};
const SDL_Color PURPLE_COLOUR	= {0xcb,0x73,0xc8,0};	//my colour
const SDL_Color GAMEBG_COLOUR	= {0xF6,0xF6,0xBD,0};	//same as bg on word_btn_xxx.png etc

//seconds per diff level ( ie diff medium = 2 so ((diff_max-2)*COUNTDOWN_xxx) )
const int COUNTDOWN_REWORD		= 60;
const int COUNTDOWN_SPEED6		= 40;
const int COUNTDOWN_TIMETRIAL	= 120;

//User defined SDL_Event codes - caught in game.cpp loop
//user event timer code
#define USER_EV_END_COUNTDOWN		1
//to sound the countdown ping
#define USER_EV_PING_COUNTDOWN		2
//Next/prev/pause music track events
#define USER_EV_NEXT_TRACK			3
#define USER_EV_PREV_TRACK			4
#define USER_EV_PAUSE_TRACK			5
#define USER_EV_STOP_TRACK			6
#define USER_EV_SAVE_STATE			7

#endif //_GAME_GLOBAL_H_

