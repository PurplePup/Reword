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

#ifndef _WIN32
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <dirent.h>		//for dir search for songs...
#endif

Audio::Audio() : _init(false), _volume(0), _volTest(0), _musicTrack(0), _lastTrack(0), _bPlayingTrack(false)
{
	_baseTrackDir = RES_BASE + "music";
}

Audio::~Audio()
{
	if (_volTest) 
		Mix_FreeChunk(_volTest);

	if (_init) Mix_CloseAudio();
}

void Audio::init()
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

#ifdef _USE_MIKMOD
    MikMod_RegisterAllDrivers();
    MikMod_RegisterAllLoaders();
    md_mode |= DMODE_SOFT_MUSIC;
    if(MikMod_Init(""))
    {
    	//setLastError("Warning: Couldn't init MikMod audio\nReason: %s\n");
        return;	//failed
    }
#endif
	
	loadTracks(_baseTrackDir);
	_init = true;
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

	//save curr volume level
	_volume = newvol;

	//play a test beep/sound at the new volume so player can tell
	if (bTest) Mix_PlayChannel(-1,_volTest,0);

}

void Audio::volumeUp()
{
	setVolume(_volume+=8);
}

void Audio::volumeDown()
{
	setVolume(_volume-=8);
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
	Audio *pAudio = Audio::instance();
	if (pAudio)
		pAudio->pushNextTrack();	//not call mix functions directly, push and exit
}

void Audio::pushNextTrack()
{
	pushEvent(USER_EV_NEXT_TRACK);
}
void Audio::pushPrevTrack()
{
	pushEvent(USER_EV_PREV_TRACK);
}
void Audio::pushPauseTrack()
{
	pushEvent(USER_EV_PAUSE_TRACK);
}
void Audio::pushStopTrack()
{
	pushEvent(USER_EV_STOP_TRACK);
}

void Audio::pushEvent(int evCode)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = evCode;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
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
	
	if (trackName.length() > 0)
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
#ifndef _WIN32
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
				if (pp_s::endsWith(sTrack, "OGG") || pp_s::endsWith(sTrack, "MP3"))
				{
					printf("%s\n", dir->d_name);
					_trackList.push_back(std::string(dir->d_name));
				}
			}
		}
		closedir(d);
	}
#else
	//load tracks on MS box
#endif
}

#ifdef _USE_MIKMOD
bool Audio::modLoad(std::string filename)
{
	char * fn = const_cast<char*>(filename.c_str());
    _modMusic = Player_Load(fn, 64, 0);
    if(!_modMusic) {
        return false;
    }
    MikMod_EnableOutput();

    return true;
}

void Audio::modStart()
{
	if (_modMusic)
	    Player_Start(_modMusic);

	//Player_SetVolume( 0..128 )
}

void Audio::modStop()
{
	if (_modMusic)
		Player_Stop();
}

void Audio::modUpdate()	//regular update call
{
	if (Player_Active()) MikMod_Update();
}
#endif

std::auto_ptr<Audio> Audio::_instance(0);

Audio * Audio::instance()			// Create an instance of the object
{
	if (_instance.get() == 0)
//		_instance.reset(new Audio());
		_instance = std::auto_ptr<Audio>(new Audio());
	return _instance.get();
}

Audio & Audio::getRef()			// Get a reference to the object
{
	return *_instance;
}

Audio * Audio::getPtr()			// Get a pointer to the object
{
	return _instance.get();
}

