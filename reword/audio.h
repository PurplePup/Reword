//audio.h

#ifndef _AUDIO_H
#define _AUDIO_H

#include "i_audio.h"    //interface

#include "error.h"
#include <SDL_mixer.h>
#include <string>
#include <deque>
#include <memory>

#ifdef _USE_MIKMOD
#include <mikmod.h>
#endif

//NOTE : NullAudio and Audio used in a Locator class

class NullAudio : public IAudio
{
public:
	virtual void init(bool bMusic, bool bSfx) { std::cout << "NullAudio (silent) initialised" << std::endl; }
	virtual void setVolume(Sint16 newvol, bool test = true) {}
	virtual void volumeUp() {}
	virtual void volumeDown() {}

	virtual void setBaseTrackDir(const std::string &baseMusicDir) {}
	virtual void startNextTrack() {}
	virtual void startPrevTrack() {}
	virtual void startTrack(const std::string &trackName) {}
	virtual void stopTrack() {}
	virtual void pauseTrack() {}
	virtual bool hasMusicTracks() { return false; }
	virtual bool isPlayingMusic() { return false; }
	virtual bool isActuallyPlayingMusic() { return false; }

	//event functions that effectively call the start/stop track functions
	virtual void pushNextTrack() {}
	virtual void pushPrevTrack() {}
	virtual void pushPauseTrack() {}
	virtual void pushStopTrack() {}
};

class Audio : public IAudio
{
public:
	Audio();
	~Audio();

	virtual void init(bool bMusic, bool bSfx);
	virtual void setVolume(Sint16 newvol, bool test = true);
	virtual void volumeUp();
	virtual void volumeDown();

	virtual void setBaseTrackDir(const std::string &baseMusicDir);
	virtual void startNextTrack();
	virtual void startPrevTrack();
	virtual void startTrack(const std::string &trackName);
	virtual void stopTrack();
	virtual void pauseTrack();
	virtual bool hasMusicTracks() { return _trackList.size() > 0; }
	virtual bool isPlayingMusic() { return _bPlayingTrack; }
	virtual bool isActuallyPlayingMusic() { return Mix_PlayingMusic()?true:false; }

	//event functions that effectively call the start/stop track functions
	virtual void pushNextTrack();
	virtual void pushPrevTrack();
	virtual void pushPauseTrack();	//toggle
	virtual void pushStopTrack();

protected:
	static void pushEvent(int evCode);

	void loadTracks(const std::string &baseDir);
	std::string getNextTrack();
	std::string getPrevTrack();

#ifdef _USE_MIKMOD
//public:
//	//MikMod functions
//	bool modLoad(std::string filename);
//	void modStart();
//	void modStop();
//	void modUpdate();
//protected:
//	MODULE		*_modMusic;
#endif

protected:
	bool		_init;
	Sint16		_volume;		//current volume setting (fx and music)
	Mix_Chunk	*_volTest;

	//music vars
	Mix_Music 	*_musicTrack;
	int			_lastTrack;
	std::string _baseTrackDir;
	std::deque<std::string> _trackList;
	bool		_bPlayingTrack;		//set true if start playing
	bool        _bMusic, _bSfx;

};

#endif //_AUDIO_H
