#if !defined I_AUDIO_H_INCLUDED
#define I_AUDIO_H_INCLUDED

#include <SDL.h>
#include <string>

class IAudio
{
public:
    IAudio() {}
	virtual ~IAudio() {}

	virtual void setup(bool bSfx, bool bMusic, const std::string &baseTrackDir, bool bMute) = 0;
	virtual void closedown() = 0;

	virtual int  getVolume() = 0;
	virtual void setVolume(Sint16 newvol, bool test = true) = 0;
	virtual void volumeUp() = 0;
	virtual void volumeDown() = 0;

    virtual void setSfxEnabled(bool bOn) = 0;
	virtual bool sfxEnabled() = 0;
	virtual void sfxMute(bool bMute = true) = 0;
    virtual bool toggleSfx() = 0;
    virtual int  getSfxVol() = 0;
    virtual void setSfxVol(Sint16 newvol, bool bTest = true) = 0;

    virtual void setMusicEnabled(bool bOn) = 0;
	virtual bool musicEnabled() = 0;
	virtual void musicMute(bool bMute = true) = 0;
    virtual bool toggleMusic(bool bIsMenu = false) = 0;
    virtual int  getMusicVol() = 0;
    virtual void setMusicVol(Sint16 newvol, bool bTest = true) = 0;

    //mute/unmute music and sfx at same time
    virtual bool isMute() = 0;
    virtual void muteAll(bool bMute = true) = 0;
    virtual bool toggleMuteAll(bool bIsMenu = false) = 0;

    virtual bool hasSound() = 0;
	virtual void setBaseTrackDir(const std::string &baseMusicDir) = 0;
	virtual void startNextTrack() = 0;
	virtual void startPrevTrack() = 0;
	virtual void startTrack(const std::string &trackName) = 0;
	virtual void stopTrack() = 0;
	virtual void pauseTrack() = 0;
	virtual bool hasMusicTracks() = 0;
	virtual bool isPlayingMusic() = 0;
	virtual bool isActuallyPlayingMusic() = 0;

    virtual int addSfx(const std::string &, unsigned int) = 0;
	virtual void playSfx(unsigned int, unsigned int = 0) = 0;
	virtual void playSfx(const std::string &, unsigned int = 0) = 0;
};

#endif // I_AUDIO_H_INCLUDED
