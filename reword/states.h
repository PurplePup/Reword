//states.h
//
// enumerated states needed in several places throughout the game
//
//

#if !defined _STATES_H
#define _STATES_H

//namespace Game
//{

    //game difficulty states
	enum eGameDiff { DIF_UNDEFINED,
                     DIF_EASY,
                     DIF_MED,
                     DIF_HARD,
                     DIF_MAX };	    //DIF_MAX only used for calc countdown timer

    //game states (menu, in-play etc)
	//ST_GAME and ST_RESUME are similar in that both start the game, but ST_RESUME loads previously saved state file
	enum eGameState { ST_MENU,      //main menu
                      ST_MODE,      //mode screen
                      ST_DIFF,      //difficulty screen
                      ST_INST,      //instructions
                      ST_GAME,      //in-play
                      ST_RESUME,    //resume menu shown
                      ST_HIGH,      //highscore screen
                      ST_HIGHEDIT,  //highscore editing
                      ST_OPTN,      //Options & prefs
                      ST_EXIT };

    //game modes (std, timetrial etc)
	enum eGameMode { GM_ARCADE,     //Reword but allows continue on partial completion
                     GM_REWORD,     //Classic Reword (must get a long word to continue)
                     GM_SPEEDER,    //Must get a single long word to continue
                     GM_TIMETRIAL,  //Get as many single long words before timer runs out
                     GM_MAX };		//GM_MAX only used to calc last entry

//}


#endif //_STATES_H
