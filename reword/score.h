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
const int SCORE_FASTEST = 2;	//each time a speed6 or TT word is got faster than before

//Make sure it's always the same packing
//This is not portable over different endian systems but will at least
//play nicer on most. If I need to be truly cross platform with the score
//file, it needs to have its own serializer or be text (XML?)
#pragma pack(push, 1)

typedef struct
{
	char	inits[4];	//3 letters + \0
	Uint32	words;
	Uint32	score;
} tHiScoreEntryPre04;		//pre v0.4 score format

typedef struct
{
	//1 more than (10) to display, to allow insert when adding inits at last pos in table
	tHiScoreEntryPre04 level[11];
} tHiScoreLevelsPre04;

typedef struct
{
	char	inits[4];	//3 letters + \0
	Uint32	words;
	Uint32	score;
	Uint32	fastest;
	Uint32	ttSeconds;	//amount of seconds - user can change for timetrial
} tHiScoreEntry;

typedef struct
{
	//1 more than (10) to display, to allow insert when adding inits at last pos in table
	tHiScoreEntry level[11];
} tHiScoreLevels;

//back to whatever the platform uses
#pragma pack(pop)

class Score
{
public:
	Score();

	void init();
	Uint32 load(std::string scorefile = "");
	void save(std::string scorefile = "");

	int isHiScore(int mode, int diffLevel);
	void insert(int mode, int diff, int pos, tHiScoreEntry &curr);

	//current score accessors
	void setCurr(tHiScoreEntry &curr);
	void setCurrInits(std::string inits);
	void setCurrWords(unsigned int words);
	unsigned int addCurrWords(unsigned int words);
	void setCurrScore(unsigned int score);
	unsigned int addCurrScore(unsigned int score);
	bool setCurrFastest(unsigned int fastest);
	void resetCurr();

	tHiScoreEntry curr() {return _curr;}
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
	void setSeed(unsigned int seed) { _seed = seed; }

protected:
	tHiScoreLevels * getLevel(int mode, int diff);

protected:
	std::string		_scorefile;

	tHiScoreEntry	_curr;			//current score, words and inits to save to scoretable
	tHiScoreLevels	_hiScoreRA[3];	//Reword Arcade - 3 difficulties (easy, med, hard) of 10x hi scores
	tHiScoreLevels	_hiScore[3];	//Reword - 3 difficulties (easy, med, hard) of 10x hi scores
	tHiScoreLevels	_hiScoreS6[3];	//Speed - 3 difficulties (easy, med, hard) of 10x hi scores
	tHiScoreLevels	_hiScoreTT[3];	//TimeTrial - 3 difficulties (easy, med, hard) of 10x hi scores

    unsigned int 	_seed;
};

//structure to hold quick save state between levels (for restore)
typedef struct
{
	Uint32	words;	//words found so far
	Uint32	score;	//
	Uint32	diff;	//easy,med,hard
	Uint32	mode;	//game type rew,speed,TT
	Uint32	seed;	//random seed to restore the word list in same order
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
