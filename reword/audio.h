//audio.h

#ifndef _AUDIO_H
#define _AUDIO_H

#include "error.h"
#include <SDL_mixer.h>
#include <string>
#include <deque>
#include <memory>

#ifdef _USE_MIKMOD
#include <mikmod.h>
#endif

//NOTE : Audio is now a singleton class

class Audio : public Error
{
protected:
	Audio();
public:
	~Audio();

    static Audio *instance();		// Create an instance of the object
    static Audio &getRef();			// Get a reference to the object
    static Audio *getPtr();			// Get a pointer to the object

	void init(bool bMusic, bool bSfx);
	void setVolume(Sint16 newvol, bool test = true);
	void volumeUp();
	void volumeDown();

	void setBaseTrackDir(const std::string &baseMusicDir);
	void startNextTrack();
	void startPrevTrack();
	void startTrack(const std::string &trackName);
	void stopTrack();
	void pauseTrack();
	bool hasMusicTracks() { return _trackList.size() > 0; }
	bool isPlayingMusic() { return _bPlayingTrack; }
	bool isActuallyPlayingMusic() { return Mix_PlayingMusic()?true:false; }

	//event functions that effectively call the start/stop track functions
	void pushNextTrack();
	void pushPrevTrack();
	void pushPauseTrack();	//toggle
	void pushStopTrack();

protected:
	static void pushEvent(int evCode);

	void loadTracks(const std::string &baseDir);
	std::string getNextTrack();
	std::string getPrevTrack();

#ifdef _USE_MIKMOD
public:
	//MikMod functions
	bool modLoad(std::string filename);
	void modStart();
	void modStop();
	void modUpdate();
protected:
	MODULE		*_modMusic;
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

private:
    static std::auto_ptr<Audio> _instance;	//singleton instance pointer

};

#endif //_AUDIO_H
