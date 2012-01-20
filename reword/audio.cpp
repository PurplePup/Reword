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
										not altering .mod file volume, just wav/mp3/ogg and fx
										Code from "Senor Quack"

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

#include "global.h"
#include "audio.h"
#include "helpers.h"
#include "utils.h"
#include "platform.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#ifndef WIN32
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <dirent.h>		//for dir search for songs...
#endif


void NullAudio::setup(bool bMusic, bool bSfx)
{
    (void)(bMusic);
    (void)(bSfx);
    std::cout << "NullAudio (silent) initialised" << std::endl;
}


Audio::Audio() : _init(false), _volTest(0), _musicTrack(0), _lastTrack(0), _bPlayingTrack(false)
{
	_baseTrackDir = RES_BASE + "music";
}

Audio::~Audio()
{
    closedown();
}

void Audio::setup(bool bMusic, bool bSfx)
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

	_volTest = Mix_LoadWAV(std::string(RES_BASE + "sounds/ping.wav").c_str()); //also used in game
	setVolume((MIX_MAX_VOLUME / 2), false); //no test sound

	_opt._bMusic = bMusic;
	_opt._bSfx = bSfx;

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

	if (_volTest)
		Mix_FreeChunk(_volTest);

	if (_init)
    {
        stopTrack();
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

	//play a test beep/sound at the new volume so player can tell
	if (bTest && _opt._bSfx) Mix_PlayChannel(-1,_volTest,0);

}

void Audio::volumeUp()
{
	setVolume(_opt._sfxVol+=8);
}

void Audio::volumeDown()
{
	setVolume(_opt._sfxVol-=8);
}

int  Audio::getSfxVol()
{
    return _opt._sfxVol;
}

bool Audio::sfxEnabled()
{
    return (_opt._bSfx && _opt._sfxVol > 0);
}

void Audio::sfxMute(bool bMute)
{
    _opt._bSfx = !bMute;
}

void Audio::setSfxVol(Sint16 newvol, bool bTest)
{
    //TODO - set individual sfx vol
    setVolume(newvol, bTest);
}

int  Audio::getMusicVol()
{
    return _opt._musicVol;
}

bool Audio::musicEnabled()
{
    return (_opt._bMusic && _opt._musicVol > 0);
}

void Audio::musicMute(bool bMute)
{
    _opt._bMusic = !bMute;
}

void Audio::setMusicVol(Sint16 newvol, bool bTest)
{
    //TODO - set individual music vol
    setVolume(newvol, bTest);
}

void Audio::setBaseTrackDir(const std::string &baseTrackDir)
{
	_baseTrackDir = baseTrackDir;
}

//global callback function for Mix_HookMusicFinished()
void AudioTrackDone()
{
	//Music has finished playing - for whatever reason
	//NOTE: NEVER call SDL_Mixer functions, nor SDL_LockAudio, from a callback function
    Locator::GetAudio().pushNextTrack();
}

void Audio::pushNextTrack()
{
	ppg::pushSDL_Event(USER_EV_NEXT_TRACK);
}
void Audio::pushPrevTrack()
{
	ppg::pushSDL_Event(USER_EV_PREV_TRACK);
}
void Audio::pushPauseTrack()
{
	ppg::pushSDL_Event(USER_EV_PAUSE_TRACK);
}
void Audio::pushStopTrack()
{
	ppg::pushSDL_Event(USER_EV_STOP_TRACK);
}

void Audio::stopTrack()
{
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
	stopTrack();	//stop any existing music

	if (_opt._bMusic && trackName.length() > 0)
	{
		printf("Play: %s\n", trackName.c_str());
		std::string newTrack = _baseTrackDir + "/" + trackName;
		_musicTrack = Mix_LoadMUS(newTrack.c_str());
		if(_musicTrack)
		{
			Mix_PlayMusic(_musicTrack, 1);
			_bPlayingTrack = Mix_PlayingMusic()?true:false;	//actualy playing?
		}
		else
			printf("Failed to start %s (%s)\n", newTrack.c_str(), Mix_GetError());

		Mix_HookMusicFinished(AudioTrackDone);	//reiterate callback
	}
	if (!_bPlayingTrack)
	{
		//TODO: play fail sound
	}
}

void Audio::pauseTrack()
{
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
	DIR           *d;
	struct dirent *dir;

	d = opendir(baseDir.c_str());
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			std::string sTrack = dir->d_name;
			if (sTrack.length()>0 && sTrack[0] != '.')
			{
				if (pptxt::endsWith(sTrack, "OGG") || pptxt::endsWith(sTrack, "MP3"))
				{
					printf("%s\n", dir->d_name);
					_trackList.push_back(std::string(dir->d_name));
				}
			}
		}
		closedir(d);
	}
#else
	//TODO - load tracks on MS box
#endif
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

