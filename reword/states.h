//states.h
//
// enumerated states needed in several places throughout the game
//
//

#ifndef _STATES_H
#define _STATES_H

//namespace Game
//{

	enum eGameDiff { DIF_UNDEFINED, DIF_EASY, DIF_MED, DIF_HARD, DIF_MAX };	//DIF_MAX only used for calc countdown timer

	//ST_GAME and ST_RESUME are similar in that both start the game, but ST_RESUME loads previously saved state file
	enum eGameState { ST_MENU, ST_MODE, ST_DIFF, ST_INST, ST_GAME, ST_RESUME, ST_HIGH, ST_HIGHEDIT, ST_EXIT };

	enum eGameMode { GM_REWORD, GM_SPEED6, GM_TIMETRIAL, GM_MAX };		//GM_MAX only used to calc last entry

//}


#endif //_STATES_H
