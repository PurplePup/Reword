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

//Null audio for disabling audio completely
class NullAudio : public IAudio
{
public:
	virtual void setup(bool bMusic, bool bSfx);
	virtual void closedown() {}

	virtual int  getVolume() { return 0; }
	virtual void setVolume(Sint16 newvol, bool test = true) { (void)(newvol); (void)(test); }
	virtual void volumeUp() {}
	virtual void volumeDown() {}

	virtual bool sfxEnabled() { return false; }
	virtual void sfxMute(bool bMute = true) { (void)(bMute); }
    virtual int  getSfxVol() { return 0; }
    virtual void setSfxVol(Sint16 newvol, bool bTest = true) { (void)(newvol);  (void)(bTest); }

	virtual bool musicEnabled() { return false; }
	virtual void musicMute(bool bMute = true) { (void)(bMute); }
    virtual int  getMusicVol() { return 0; }
    virtual void setMusicVol(Sint16 newvol, bool bTest = true) { (void)(newvol);  (void)(bTest); }

    virtual bool isMute() { return true; }
    virtual void mute(bool bMute = true) { (void)(bMute); }

    virtual bool hasSound() { return false; }   //always, as this IS NUllAudio
	virtual bool hasMusicTracks() { return false; }
	virtual void setBaseTrackDir(const std::string &baseMusicDir) { (void)(baseMusicDir); }
	virtual void startNextTrack() {}
	virtual void startPrevTrack() {}
	virtual void startTrack(const std::string &trackName) { (void)(trackName); }
	virtual void stopTrack() {}
	virtual void pauseTrack() {}
	virtual bool isPlayingMusic() { return false; }
	virtual bool isActuallyPlayingMusic() { return false; }

	//event functions that effectively call the start/stop track functions
	virtual void pushNextTrack() {}
	virtual void pushPrevTrack() {}
	virtual void pushPauseTrack() {}
	virtual void pushStopTrack() {}
};

//options and settings for audio volume etc
struct AudioOptions
{
    AudioOptions()
//        : _bMusic(true), _musicVol(50) _bSfx(true), _sfxVol(50)
    {
        _bMusic = _bSfx = true;
        _musicVol = _sfxVol = 50;
    }

    bool        _bMusic;
    Sint16		_musicVol;      //music vol
    bool        _bSfx;
    Sint16		_sfxVol;        //sfx vol
};

//Standard audio implementation
class Audio : public IAudio
{
public:
	Audio();
	~Audio();
	virtual void setup(bool bMusic, bool bSfx);
	virtual void closedown();

	virtual int  getVolume();
	virtual void setVolume(Sint16 newvol, bool bTest = true);
	virtual void volumeUp();
	virtual void volumeDown();

	virtual bool sfxEnabled();
	virtual void sfxMute(bool bMute = true);
    virtual int  getSfxVol();
    virtual void setSfxVol(Sint16 newvol, bool bTest = true);

	virtual bool musicEnabled();
	virtual void musicMute(bool bMute = true);
    virtual int  getMusicVol();
    virtual void setMusicVol(Sint16 newvol, bool bTest = true);

    virtual bool isMute();
    virtual void mute(bool bMute = true);

    virtual bool hasSound() { return true; }    //always as this is NOT NullAudio
	virtual bool hasMusicTracks() { return _trackList.size() > 0; }
	virtual void setBaseTrackDir(const std::string &baseMusicDir);
	virtual void startNextTrack();
	virtual void startPrevTrack();
	virtual void startTrack(const std::string &trackName);
	virtual void stopTrack();
	virtual void pauseTrack();
	virtual bool isPlayingMusic() { return _bPlayingTrack; }
	virtual bool isActuallyPlayingMusic() { return Mix_PlayingMusic(); }

	//event functions that effectively call the start/stop track functions
	virtual void pushNextTrack();
	virtual void pushPrevTrack();
	virtual void pushPauseTrack();	//toggle
	virtual void pushStopTrack();

protected:
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
	Mix_Chunk	*_volTest;

	//music track vars
	Mix_Music 	*_musicTrack;
	int			_lastTrack;
	std::string _baseTrackDir;
	std::deque<std::string> _trackList;
	bool		_bPlayingTrack;		//set true if start playing

    AudioOptions _opt;
};

#endif //_AUDIO_H
