//score.h

#ifndef _SCORE_H
#define _SCORE_H

#include "SDL.h"
#include <string>
#include <fstream>
#include <ios>

#include "states.h"

//score vaues
const int SCORE_ARCADE = 30;    //bonus for getting enough words (but not the all-letter word)
const int SCORE_BONUS = 100;	//given when word level completed
const int SCORE_SECONDS = 10;	//multiplied by seconds remaining and added to bonus
const int SCORE_WORD6 = 20;		//for all all-letter letter words
const int SCORE_WORD = 10;		//for each non all-letter word
const int SCORE_FASTEST = 2;	//each time a speeder or TT word is got faster than before

//Make sure it's always the same packing
//This is not portable over different endian systems but will at least
//play nicer on most. If I need to be truly cross platform with the score
//file, it needs to have its own serializer or be text (XML/JSON/INI etc?)
#pragma pack(push, 1)

struct HiScoreEntryPre04
{
    HiScoreEntryPre04() : words(0), score(0) { memset(inits, 0, 4); }
	char	inits[4];	//3 letters + \0
	Uint32	words;
	Uint32	score;
};		//pre v0.4 score format

struct HiScoreLevelsPre04
{
	//1 more than (10) to display, to allow insert when adding inits at last pos in table
	HiScoreEntryPre04 level[11];
};

struct HiScoreEntry
{
    HiScoreEntry() : words(0), score(0), fastest(0), ttSeconds(0) { memset(inits, 0, 4); }
	char	inits[4];	//3 letters + \0
	Uint32	words;
	Uint32	score;
	Uint32	fastest;
	Uint32	ttSeconds;	//amount of seconds - user can change for timetrial
};

struct HiScoreLevels
{
	//1 more than (10) to display, to allow insert when adding inits at last pos in table
	HiScoreEntry level[11];
};

//back to whatever the platform uses
#pragma pack(pop)

class Score
{
public:
	Score();

	void init();

    Uint32 loadUsingWordfileName(const std::string &wordfile);
	Uint32 load(const std::string &scorefile = "");
	void save(const std::string &scorefile = "");

	int isHiScore(int mode, int diffLevel);
	void insert(int mode, int diff, int pos, HiScoreEntry &scoreEntry);

	//current score accessors
	void setCurr(HiScoreEntry &newCurr);
	void setCurrInits(std::string newInits);
	void setCurrWords(unsigned int newWords);
	unsigned int addCurrWords(unsigned int newWords);
	void setCurrScore(unsigned int newScore);
	unsigned int addCurrScore(unsigned int newScore);
	bool setCurrFastest(unsigned int newFastest);
	void resetCurr();

	HiScoreEntry curr() {return _curr;}
	std::string currInits() {return std::string(_curr.inits);}
	unsigned int currWords() {return _curr.words;}
	unsigned int currScore() {return _curr.score;}
	unsigned int currFastest() {return _curr.fastest;}

	//score table accessors
	std::string inits(int mode, int diff, int level);
	unsigned int words(int mode, int diff, int level);
	unsigned int score(int mode, int diff, int level);
	unsigned int fastest(int mode, int diff, int level);
	unsigned int seed() { return _seed; }
	void setSeed(unsigned int newSeed) { _seed = newSeed; }

protected:
	HiScoreLevels * getLevel(int mode, int diff);

protected:
	std::string		_scorefile;

	HiScoreEntry	_curr;			//current score, words and inits to save to scoretable
	HiScoreLevels	_hiScoreRA[3];	//Reword Arcade - 3 difficulties (easy, med, hard) of 10x hi scores
	HiScoreLevels	_hiScore[3];	//Reword - 3 difficulties (easy, med, hard) of 10x hi scores
	HiScoreLevels	_hiScoreS6[3];	//Speed - 3 difficulties (easy, med, hard) of 10x hi scores
	HiScoreLevels	_hiScoreTT[3];	//TimeTrial - 3 difficulties (easy, med, hard) of 10x hi scores

    unsigned int 	_seed;
};

//structure to hold quick save state between levels (for restore)
typedef struct qss
{
    qss() : _words(0), _score(0), _diff(0), _mode(0), _seed(0) {}
    std::string _wordfile;
	Uint32	    _words;	//words found so far
	Uint32	    _score;	//
	Uint32	    _diff;	//easy,med,hard
	Uint32	    _mode;	//game type rew,speed,TT
	Uint32	    _seed;	//random seed to restore the word list in same order
} tQuickStateSave;

class QuickState
{
public:
	QuickState();
    bool quickStateLoad();
    bool quickStateSave();
	bool quickStateExists();
	void quickStateDelete();
	void setQuickState(const tQuickStateSave &qss);
	void getQuickState(tQuickStateSave &qss);

protected:
	std::string		_quickstatefile;
 	tQuickStateSave	_qStateSave;

};

#endif //_SCORE_H
