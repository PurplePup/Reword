////////////////////////////////////////////////////////////////////
/*

File:			audio.cpp

Class impl:		Audio

Description:	A class to manage the SDL audio interface
				It was intended to use the Singleton template class to create it,
				but some compilers (VC6) can't handle it so haven't used it (yet)

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.5		18.06.2008	Changed volume control to use /dev/mixer as SDL_mixer
									not altering .mod volume, just wav/mp3/ogg and fx.
									Code from "Senor Quack"
				0.7		02.01.17	Moved to SDL2

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

#include "audio.h"

#include "global.h"
#include "helpers.h"
#include "utils.h"
#include "platform.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>

#if !defined WIN32
#include <sys/ioctl.h>
#include <unistd.h> //for close()
#include <linux/soundcard.h>
#include <dirent.h>		//for dir search for songs...
#endif

#include <SDL_mixer.h>

/////////////////////////////////////////////////////////////////////////////////////////////
//
//  NullAudio - single non-header fn
//
void NullAudio::setup(bool, bool, const std::string &, bool)
{
    std::cout << "NullAudio (silent) initialised" << std::endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////


Audio::Audio() : _init(false), _volTest(0), _musicTrack(0), _lastTrack(0), _bPlayingTrack(false)
{
	_baseTrackDir = RES_MUSIC;
	_sfxVolSave = _musicVolSave = 0;
}

Audio::~Audio()
{
    closedown();
}
void Audio::setBaseTrackDir(const std::string &baseTrackDir)
{
	_baseTrackDir = baseTrackDir;
}

//pass in default options for music and sfx from options screen, and any mute option from
//command line, which overrides music and sfx options.
void Audio::setup(bool bSfx, bool bMusic, const std::string &baseTrackDir, bool bMute)
{
	if (_init) return;

	//open audio with chunksize of 128 for gp2x, as smaller this is,
	//the more often the sound hooks will be called, reducing lag
	//e.g Mix_OpenAudio(22050, AUDIO_S16, 2, 2048)
	if (Mix_OpenAudio(SOUND_FREQUENCY, SOUND_FORMAT, SOUND_CHANELS, SOUND_CHUNK) == -1)
	{
		//setLastError("Warning: Couldn't set audio\nReason: %s\n");
		return;	//not set _init
	}

	_opt._bMusic = bMusic && !bMute;  //music on or off
	_opt._bSfx = bSfx && !bMute;      //sfx on or off

	_volTest = Mix_LoadWAV(std::string(RES_SOUNDS + "ping.wav").c_str()); //also used in game
	setVolume((MIX_MAX_VOLUME / 2), false); //no test sound

    musicMute(!bMusic);
    sfxMute(!bSfx);

//#ifdef _USE_MIKMOD
//    if (_gd->_options._bMusic)
//    {
//      MikMod_RegisterAllDrivers();
//      MikMod_RegisterAllLoaders();
//      md_mode |= DMODE_SOFT_MUSIC;
//      if(MikMod_Init(""))
//      {
//    	    //setLastError("Warning: Couldn't init MikMod audio\nReason: %s\n");
//          return;	//failed
//      }
//      std::cout << "Using MikMod audio directly" << std::endl;
//      modStart();
//#endif //_USE_MIKMOD

	setBaseTrackDir(baseTrackDir);
    if (bMusic)
        loadTracks(_baseTrackDir);

    std::cout << "Standard audio initialised" << std::endl;
	_init = true;
}

void Audio::closedown()
{
//#ifdef _USE_MIKMOD
//	if (_bMusic) modStop();
//#endif

    stopTrack();

	Mix_FreeChunk(_volTest);
    Mix_FreeMusic(_musicTrack);

    std::vector<Mix_Chunk *>::iterator it = _sfxList.begin();
    std::vector<Mix_Chunk *>::iterator end = _sfxList.end();
    for ( ; it!=end; ++it)
    {
        Mix_FreeChunk((*it));
        (*it) = nullptr;
    }
    _sfxList.clear();

	if (_init)
	{
        Mix_CloseAudio();
	}

    _init = false;
}

/*
code from gp2x forum from "Senor Quack" works with all sound, so .mod too
//DKS - newstatic
void gp2x_set_volume(int newvol)
{
	if ((newvol >= 0) && (newvol <= 100))
	{
		unsigned long soundDev = open("/dev/mixer", O_RDWR);
		int vol = ((newvol << 8) | newvol);
		ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
		close(soundDev);
	}
}

// Returns 0-100, current mixer volume, -1 on error.
static int gp2x_get_volume(void)
{
        int vol = -1;
    unsigned long soundDev = open("/dev/mixer", O_RDONLY);
    if (soundDev)
    {
        ioctl(soundDev, SOUND_MIXER_READ_PCM, &vol);
        close(soundDev);
        if (vol != -1) {
                 //just return one channel , not both channels, they're hopefully the same anyways
                 return (vol & 0xFF);
        }
    }
        return vol;
}
*/

int Audio::getVolume()
{
    //curr only using single vol level
    return _opt._sfxVol;
}

void Audio::setVolume(Sint16 newvol, bool bTest)
{
#if defined(WIN32)
	//support volume change, but SDL not support .mod file volume
	if (newvol < 0) newvol = 0;
	if (newvol > MIX_MAX_VOLUME) newvol = MIX_MAX_VOLUME;	//SDL volume min/max

	Mix_Volume(-1,newvol);
//	Mix_VolumeMusic(mVolume);
#else
	//use /dev/mixer instead of SDL as it's for the whole mixer device, inc .mod playback
	if (newvol < 0) newvol = 0;
	if (newvol > 100) newvol = 100;
	unsigned long soundDev = open("/dev/mixer", O_RDWR);
	int vol = ((newvol << 8) | newvol);
	ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
	close(soundDev);
#endif

    //currently the same vol as overall volume...
    _opt._sfxVol = _opt._musicVol = newvol;
    //if we're setting an actual volume (not 0/mute) then save for unmuting
	if (newvol > 0) _sfxVolSave = _musicVolSave = newvol;

	//play a test beep/sound at the new volume so player can tell
	if (bTest && !isMute()) Mix_PlayChannel(-1,_volTest,0);
}

/*
void Audio::playSfx(int channel, Mix_Chunk *chuk, int loops)
{
    if (_bMute || !_opt._bSfx) return;
	Mix_PlayChannel(channel, chunk, loops);
}

void Audio::playMusic(int channel, Mix_Chunk *chuk, int loops)
{
    if (_bMute || !_opt._bSfx) return;
	Mix_PlayChannel(channel, chunk, loops);
}
*/

void Audio::volumeUp()
{
    if (!isMute())
        setVolume(_opt._sfxVol+=8);
}

void Audio::volumeDown()
{
    if (!isMute())
        setVolume(_opt._sfxVol-=8);
}

bool Audio::sfxEnabled()
{
    //std::cout << "sfxEnabled() = " << (_opt._bSfx && _opt._sfxVol > 0) << std::endl;
    return (_opt._bSfx && _opt._sfxVol > 0);
}

void Audio::sfxMute(bool bMute)
{
    if (bMute)  //turn mute on
    {
        _sfxVolSave = getSfxVol();
        setSfxVol(0, false);
    }
    else    //unmute
    {
        setSfxVol(_sfxVolSave, false);
    }
    _opt._bSfx = !bMute;
 }

bool Audio::toggleSfx()
{
    bool bIsMute = _opt._sfxVol == 0;
    sfxMute(!bIsMute);
    return bIsMute;   //return prev state
}

int  Audio::getSfxVol()
{
    return _opt._sfxVol;
}

void Audio::setSfxVol(Sint16 newvol, bool bTest)
{
    //TODO - set individual sfx vol
    setVolume(newvol, bTest);
}

bool Audio::musicEnabled()
{
    //std::cout << "musicEnabled() = " << (_opt._bMusic && _opt._musicVol > 0) << std::endl;
    return (_opt._bMusic && _opt._musicVol > 0);
}

void Audio::musicMute(bool bMute)
{
    if (bMute)  //turn mute on
    {
        _musicVolSave = getMusicVol();
        setMusicVol(0, false);
        if (Mix_PlayingMusic() && !Mix_Paused(-1))
            Mix_PauseMusic();
    }
    else    //unmute
    {
        setMusicVol(_musicVolSave, false);
        Mix_ResumeMusic();
    }
    _opt._bMusic = !bMute;
}

bool Audio::toggleMusic(bool bIsMenu)
{
    bool bIsMute = !musicEnabled();
    musicMute(!bIsMute);
    if (bIsMenu)
        ppg::pushSDL_Event(USER_EV_START_MENU_MUSIC);   //start the menu music
    return bIsMute;   //return prev state
}

int  Audio::getMusicVol()
{
    return _opt._musicVol;
}

void Audio::setMusicVol(Sint16 newvol, bool bTest)
{
    //TODO - set individual music vol
    setVolume(newvol, bTest);
}

//mute both sfx and music. Seperate mute flag, overrides.
bool Audio::isMute()
{
    return musicEnabled() && sfxEnabled();
}

//mute both sfx and music at same time
void Audio::muteAll(bool bMute /*= true*/)
{
    musicMute(bMute);   //in case musicEnabled() used instead of isMute()
    sfxMute(bMute);   //in case sfxEnabled() used instead of isMute()
}

bool Audio::toggleMuteAll(bool bIsMenu /*= false*/)
{
    muteAll(!isMute());    //toggle

    if (bIsMenu)
        ppg::pushSDL_Event(USER_EV_START_MENU_MUSIC);   //start the menu music

    return !isMute();   //return prev state
}

//global callback function for Mix_HookMusicFinished()
void AudioTrackDone()
{
	//Music has finished playing - for whatever reason
	//NOTE: NEVER call SDL_Mixer functions, nor SDL_LockAudio, from a callback function
    ppg::pushSDL_Event(USER_EV_START_NEXT_TRACK);
}

void Audio::stopTrack()
{
    //std::cout << "stopTrack()" << std::endl;
	_bPlayingTrack = false;
	Mix_HaltMusic();	//fires AudioTrackDone()
}

void Audio::startNextTrack()
{
	startTrack(getNextTrack());
}

void Audio::startPrevTrack()
{
	startTrack(getPrevTrack());
}

void Audio::startTrack(const std::string &trackName)
{
    //std::cout << "startTrack()" << std::endl;
	stopTrack();	//stop any existing music

	if (_opt._bMusic && trackName.length() > 0)
	{
		printf("Play: %s\n", trackName.c_str());
		std::string newTrack = _baseTrackDir + "/" + trackName;
        Mix_FreeMusic(_musicTrack);
		_musicTrack = Mix_LoadMUS(newTrack.c_str());
		if(_musicTrack)
		{
			Mix_PlayMusic(_musicTrack, 1);
			_bPlayingTrack = isActuallyPlayingMusic();	//actualy playing?
		}
		else
			printf("Failed to start track %s (%s)\n", newTrack.c_str(), Mix_GetError());

		Mix_HookMusicFinished(AudioTrackDone);	//reiterate callback
	}
	if (!_bPlayingTrack)
	{
		//TODO: play fail sound
	}
}

void Audio::pauseTrack()
{
    //std::cout << "pauseTrack()" << std::endl;
    if (!_opt._bMusic) return;

    if (Mix_Paused(-1))
        Mix_ResumeMusic();
    else
        Mix_PauseMusic();
}

std::string Audio::getNextTrack()
{
	if (_trackList.size() == 0) return "";	//no tracks

	if (++_lastTrack > (int)_trackList.size()) _lastTrack=1;
	return _trackList[_lastTrack-1];
}

std::string Audio::getPrevTrack()
{
	if (_trackList.size() == 0) return "";	//no tracks

	if (--_lastTrack < 1) _lastTrack=_trackList.size();
	return _trackList[_lastTrack-1];
}

void Audio::loadTracks(const std::string &baseDir)
{
#ifndef WIN32
	struct dirent *dir;
	DIR *d = opendir(baseDir.c_str());
	if (d)
	{
		while ((dir = readdir(d)) != nullptr)
		{
			std::string sTrack = dir->d_name;
			if (sTrack.length()>0 && sTrack[0] != '.')
			{
				if (pptxt::endsWith(sTrack, "OGG") || pptxt::endsWith(sTrack, "MP3"))
				{
					printf("%s\n", sTrack.c_str());
					_trackList.push_back(sTrack);
				}
			}
		}
		closedir(d);
	}
#else
	//TODO - load tracks on MS box
#endif
}

//add a sound effect to the resource list and check required position is correct
int Audio::addSfx(const std::string &filename, unsigned int reqPos)
{
    Mix_Chunk *pMix = Mix_LoadWAV(std::string(RES_SOUNDS + filename).c_str());
    if (!pMix) return -1;

    _sfxList.push_back(pMix);
    const bool bOk= (_sfxList.size() == reqPos+1);

#if _DEBUG
std::cout << "# of sounds " << _sfxList.size() << " - added " << filename << (bOk?" ok":" failed") << std::endl;
#endif

    return bOk?reqPos:-1;
}

//fast lookup and play using sound index
void Audio::playSfx(unsigned int iSnd, unsigned int count)
{
    if (iSnd < _sfxList.size() && sfxEnabled())
        Mix_PlayChannel(-1, _sfxList[iSnd], count);
}

//slower lookup using original name of sound file
void Audio::playSfx(const std::string &sound, unsigned int count)
{
    (void)(sound);
    (void)(count);
    //********NOT IMPLEMENTED YET********
//    if (sfxEnabled())
//        Mix_PlayChannel(-1, _volTest, 0);
}

#ifdef _USE_MIKMOD
//bool Audio::modLoad(std::string filename)
//{
//	char * fn = const_cast<char*>(filename.c_str());
//    _modMusic = Player_Load(fn, 64, 0);
//    if(!_modMusic) {
//        return false;
//    }
//    MikMod_EnableOutput();
//
//    return true;
//}
//
//void Audio::modStart()
//{
//	if (_modMusic)
//	    Player_Start(_modMusic);
//
//	//Player_SetVolume( 0..128 )
//}
//
//void Audio::modStop()
//{
//	if (_modMusic)
//		Player_Stop();
//}
//
//void Audio::modUpdate()	//regular update call
//{
//	if (Player_Active()) MikMod_Update();
//}
#endif  //_USE_MIKMOD

