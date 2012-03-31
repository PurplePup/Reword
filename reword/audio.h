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
	virtual void setup(bool bSfx, bool bMusic, const std::string &baseTrackDir, bool bMute);
	virtual void closedown() {}

	virtual int  getVolume() { return 0; }
	virtual void setVolume(Sint16 newvol, bool test = true) { (void)(newvol); (void)(test); }
	virtual void volumeUp() {}
	virtual void volumeDown() {}

    virtual void setSfxEnabled(bool bOn) { (void)(bOn); }
	virtual bool sfxEnabled() { return false; }
	virtual void sfxMute(bool bMute = true) { (void)(bMute); }
    virtual bool toggleSfx() { return false; }
    virtual int  getSfxVol() { return 0; }
    virtual void setSfxVol(Sint16 newvol, bool bTest = true) { (void)(newvol);  (void)(bTest); }

    virtual void setMusicEnabled(bool bOn) { (void)(bOn); }
	virtual bool musicEnabled() { return false; }
	virtual void musicMute(bool bMute = true) { (void)(bMute); }
    virtual bool toggleMusic(bool bIsMenu = false) { (void)(bIsMenu); return false; }
    virtual int  getMusicVol() { return 0; }
    virtual void setMusicVol(Sint16 newvol, bool bTest = true) { (void)(newvol);  (void)(bTest); }

    virtual bool isMute() { return true; }
    virtual void muteAll(bool bMute = true) { (void)(bMute); }
    virtual bool toggleMuteAll(bool bIsMenu = false) { (void)(bIsMenu); return true; }

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
};

//options and settings for in-game working audio settings
struct AudioOptions
{
    AudioOptions()
    {
        _bMusic = _bSfx = true;     //available/mute or not
        _musicVol = _sfxVol = 50;
    }

    bool        _bMusic;          //music on/off
    Sint16		_musicVol;        //music vol (0==muted)
    bool        _bSfx;            //sfx on/off
    Sint16		_sfxVol;          //sfx vol (0==muted)
};

//Standard audio implementation
class Audio : public IAudio
{
public:
	Audio();
	~Audio();
	virtual void setup(bool bSfx, bool bMusic, const std::string &baseTrackDir, bool bMute);
	virtual void closedown();

	virtual int  getVolume();
	virtual void setVolume(Sint16 newvol, bool bTest = true);
	virtual void volumeUp();
	virtual void volumeDown();

    virtual void setSfxEnabled(bool bOn) { _opt._bSfx = bOn; }
	virtual bool sfxEnabled();
	virtual void sfxMute(bool bMute = true);
    virtual bool toggleSfx();
    virtual int  getSfxVol();
    virtual void setSfxVol(Sint16 newvol, bool bTest = true);

    virtual void setMusicEnabled(bool bOn) { _opt._bMusic = bOn; }
	virtual bool musicEnabled();
	virtual void musicMute(bool bMute = true);
    virtual bool toggleMusic(bool bIsMenu = false);
    virtual int  getMusicVol();
    virtual void setMusicVol(Sint16 newvol, bool bTest = true);

    virtual bool isMute();
    virtual void muteAll(bool bMute = true);
    virtual bool toggleMuteAll(bool bIsMenu = false);

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
    Sint16		_sfxVolSave, _musicVolSave;  //vol prev to mute

};

#endif //_AUDIO_H
