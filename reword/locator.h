#ifndef LOCATOR_H
#define LOCATOR_H

//several classes are handled by the Locator
//Audio, GameData, Input etc

#include "audio.h"
#include "input.h"
#include "image.h"
#include "resource.h"
//#include "gamedata.h"

#include <cassert>

class Locator
{
//AUDIO
public:
    static void     initAudio();
    static IAudio&  audio();
    static void     registerAudio(IAudio* audio);

//INPUT
public:
    static void     initInput();
    static IInput&  input();
    static void     registerInput(IInput* input);


/*
//GAMEDATA
public:
    static void     initData();
    static IGameData&  data();
    static void     registerdata(IGameData* data);
*/
};


#endif // LOCATOR_H
