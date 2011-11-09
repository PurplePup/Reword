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

//////////////////////////////// AUDIO /////////////////////////////

static IAudio * _audio = 0;
static NullAudio _nullaudio;

void Locator::InitAudio()
{
    _audio = &_nullaudio;
}
IAudio& Locator::GetAudio()
{
    assert(_audio != NULL);
    return *_audio;
}
void Locator::RegisterAudio(IAudio* audio)
{
    if (audio == NULL)
        _audio = &_nullaudio;   // revert to null service
    else
        _audio = audio;
}

/*
//////////////////////////////// INPUT /////////////////////////////

static IInput * _input = 0;
static NullInput _nullinput;

void Locator::InitInput()
{
    _input = &_nullinput;
}
IInput& Locator::GetInput()
{
    assert(_input != NULL);
    return *_input;
}
void Locator::RegisterInput(IInput* input)
{
    if (input == NULL)
        _input = &_nullinput;   // revert to null service
    else
        _input = input;
}

//////////////////////////////// DATA /////////////////////////////

static IGamedata * _data = 0;
static NullGamedata _nulldata;

void Locator::InitData()
{
    _data = &_nulldata;
}
IInput& Locator::GetData()
{
    assert(_data != NULL);
    return *_data;
}
void Locator::Registerdata(IGameData* data)
{
    if (data == NULL)
        _data = &_nulldata;   // revert to null service
    else
        _data = data;
}
*/