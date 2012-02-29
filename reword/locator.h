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
    static void     InitAudio();
    static IAudio&  GetAudio();
    static void     RegisterAudio(IAudio* audio);

//INPUT
public:
    static void     InitInput();
    static IInput&  GetInput();
    static void     RegisterInput(IInput* input);


/*
//GAMEDATA
public:
    static void     InitData();
    static IGameData&  GetData();
    static void     Registerdata(IGameData* data);
*/
};


#endif // LOCATOR_H
