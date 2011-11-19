#ifndef I_AUDIO_H_INCLUDED
#define I_AUDIO_H_INCLUDED

#include <SDL_mixer.h>
#include <string>

class IAudio
{
public:
	virtual void init(bool bMusic, bool bSfx) = 0;
	virtual void setVolume(Sint16 newvol, bool test = true) = 0;
	virtual void volumeUp() = 0;
	virtual void volumeDown() = 0;

	virtual void setBaseTrackDir(const std::string &baseMusicDir) = 0;
	virtual void startNextTrack() = 0;
	virtual void startPrevTrack() = 0;
	virtual void startTrack(const std::string &trackName) = 0;
	virtual void stopTrack() = 0;
	virtual void pauseTrack() = 0;
	virtual bool hasMusicTracks() = 0;
	virtual bool isPlayingMusic() = 0;
	virtual bool isActuallyPlayingMusic() = 0;

	//event functions that effectively call the start/stop track functions
	virtual void pushNextTrack() = 0;
	virtual void pushPrevTrack() = 0;
	virtual void pushPauseTrack() = 0;	//toggle
	virtual void pushStopTrack() = 0;
};

#endif // I_AUDIO_H_INCLUDED
