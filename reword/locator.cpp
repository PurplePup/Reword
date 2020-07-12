////////////////////////////////////////////////////////////////////
/*

File:			locator.cpp

Class impl:		Locator

Description:	A Service Locator class for Audio, Input, GameData
                To register important service classes and return pointers
                or to register null services when not required

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			09 Nov 2011

Licence:		This program is free software; you can redistribute it and/or modify
				it under the terms of the GNU General Public License as published by
				the Free Software Foundation; either version 2 of the License, or
				(at your option) any later version.

				This software is distributed in the hope that it will be useful,
				but WITHOUT ANY WARRANTY; without even the implied warranty of
				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
				GNU General Public License for more details.

				You should have received a copy of the GNU General Public License
				along with this program; if not, write to the Free Software
				Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
////////////////////////////////////////////////////////////////////


#include "locator.h"
#include "gamedata.h"

#include <cassert>

//////////////////////////////// AUDIO /////////////////////////////

static IAudio * _audio = nullptr;
static NullAudio _nullaudio;

void Locator::initAudio()
{
    _audio = &_nullaudio;
}
IAudio& Locator::audio()
{
    assert(_audio != nullptr);
    return *_audio;
}
void Locator::registerAudio(IAudio* audio)
{
    if (audio == nullptr)
        _audio = &_nullaudio;   // revert to null service
    else
        _audio = audio;
}


//////////////////////////////// SCREEN /////////////////////////////

static Screen * _screen = nullptr;
static Screen _nullscreen;

void Locator::initScreen()
{
    _screen = &_nullscreen;
}
Screen& Locator::screen()
{
    assert(_screen != nullptr);
    return *_screen;
}
void Locator::registerScreen(Screen* screen)
{
    if (screen == nullptr)
        _screen = &_nullscreen;   // revert to null service
    else
        _screen = screen;
}


//////////////////////////////// INPUT /////////////////////////////

static IInput * _input = nullptr;
static NullInput _nullinput;

void Locator::initInput()
{
    _input = &_nullinput;
}
IInput& Locator::input()
{
    assert(_input != nullptr);
    return *_input;
}
void Locator::registerInput(IInput* input)
{
    if (input == nullptr)
        _input = &_nullinput;   // revert to null service
    else
        _input = input;
}


//////////////////////////////// DATA /////////////////////////////

static GameData * _data = nullptr;
static GameData _nulldata;

void Locator::initData()
{
    _data = &_nulldata;
}
GameData& Locator::data()
{
    assert(_data != nullptr);
    return *_data;
}
void Locator::registerData(GameData* data)
{
    if (data == nullptr)
        _data = &_nulldata;   // revert to null service
    else
        _data = data;
}

