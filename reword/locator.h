#if !defined LOCATOR_H
#define LOCATOR_H

//several classes are handled by the Locator
//Audio, GameData, Input etc

#include "audio.h"
#include "screen.h"
#include "input.h"
#include "image.h"
#include "resource.h"
#include "gamedata.h"

#include <cassert>

//class GameData;

class Locator
{
//AUDIO
public:
    static void     initAudio();
    static IAudio&  audio();
    static void     registerAudio(IAudio* audio);

//SCREEN
public:
    static void     initScreen();
    static Screen&  screen();
    static void     registerScreen(Screen* screen);

//INPUT
public:
    static void     initInput();
    static IInput&  input();
    static void     registerInput(IInput* input);

//GAMEDATA
public:
    static void     initData();
    static GameData&  data();
    static void     registerData(GameData* data);

};


#endif // LOCATOR_H
