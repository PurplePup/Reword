//platform.h
//

#if defined(WIN32)
//win32 pragmas to help prevent STL truncation warnings
#pragma warning(disable:4503)
#pragma warning(disable:4786)
#pragma warning(disable:4788)
//MSVC compiler + runtime
#define round(x) floor(x+0.5)
#define strcasecmp(s1,s2) stricmp(s1,s2)
#define snprintf _snprintf
#else
//POSIX - GCC compiler
#endif

//base data location depending on system
#if defined(WIN32)
#define RES_SYS_BASE	(std::string("../data/"))
#else
#define RES_SYS_BASE	(std::string("./data/"))
#endif


//screen and hardware dependant settings
#if defined (GP2X)
//GPH GP2X
#define DEBUG_HW_NAME	"GP2X"
#define RES_BASE		(RES_SYS_BASE + std::string("gp2x/"))
#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	240
#define SCREEN_SURFACE	SDL_SWSURFACE | SDL_FULLSCREEN
#define SCREEN_BPP		16
#define SOUND_FREQUENCY	22050
#define SOUND_FORMAT	AUDIO_S16
#define SOUND_CHANELS	2
#define SOUND_CHUNK		128
#define CURSORW			40
#define CURSORH			40
#define LETTERW			36
#define LETTERH			36
#define BOXW			90
#define BOXH			15
#define BOXHGAP         3
#define BOXTEXTOFFSETX	4
#define BOXTEXTOFFSETY	4
#define MENU_HI_X		58
#define MENU_HI_Y		55
#define MENU_HI_GAP		30
#define MENU_HI_OFF		7
#define FONT_TINY		11
#define FONT_SMALL		14
#define FONT_MEDIUM		16
#define FONT_BIG		20
#define FONT_CLEAN		14
#define FONT_CLEAN_MAX	40
#define ROUNDEL_VEL		12
#define BG_LINE_TOP		(LETTERH+10)
#define BG_LINE_BOT		(SCREEN_HEIGHT-30)
#define GAME_GAP1		3
#define MAX_WORD_COL	8	//max number of words to display down a 3, 4, 5 or 6 word column
#define MAX_WORD_ROW	4	//4 across screen, 3,4,5,6 for 6, or 5,6,7,8 for 8 letter target word
#define FOUND_WORD_CHR	10	//pixels wide for calculating found word box size

#elif defined (PANDORA)
//OpenPandora UMPC
#define DEBUG_HW_NAME	"PANDORA"
#define RES_BASE		(RES_SYS_BASE + std::string("pandora/"))
#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	480
#define SCREEN_SURFACE	SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN
//#define SCREEN_SURFACE	SDL_SWSURFACE|SDL_FULLSCREEN
#define SCREEN_BPP		32
#define SOUND_FREQUENCY	22050
#define SOUND_FORMAT	AUDIO_S16
#define SOUND_CHANELS	2
#define SOUND_CHUNK		128
#define CURSORW			76
#define CURSORH			76
#define LETTERW			72
#define LETTERH			72
#define BOXW			144
#define BOXH			25
#define BOXHGAP         3
#define BOXTEXTOFFSETX	8
#define BOXTEXTOFFSETY	-2
#define MENU_HI_X		58
#define MENU_HI_Y		(LETTERH + 40)
#define MENU_HI_GAP		55
#define MENU_HI_OFF		20
#define FONT_SMALL		20
#define FONT_MEDIUM		26
#define FONT_BIG		34
#define FONT_TINY		18
#define FONT_CLEAN		22
#define FONT_CLEAN_MAX	80
#define ROUNDEL_VEL		20
#define BG_LINE_TOP		85
#define BG_LINE_BOT		415
#define GAME_GAP1		8
#define MAX_WORD_COL	8	//max number of words to display down a 3, 4, 5 or 6 word column
#define MAX_WORD_ROW	4	//4 across screen, 3,4,5,6 for 6, or 5,6,7,8 for 8 letter target word
#define FOUND_WORD_CHR	18	//pixels wide for calculating found word box size

//#elif defined (WIZ)

#else

#define DEBUG_HW_NAME	"PC"
//default 'PC' Linux or Win
#define RES_BASE		(RES_SYS_BASE + std::string("pandora/"))
#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	480
//#define SCREEN_SURFACE	SDL_HWSURFACE|SDL_DOUBLEBUF
#define SCREEN_SURFACE	SDL_SWSURFACE
#define SCREEN_BPP		32
#define SOUND_FREQUENCY	22050
#define SOUND_FORMAT	AUDIO_S16
#define SOUND_CHANELS	2
#define SOUND_CHUNK		2048
#define CURSORW			76
#define CURSORH			76
#define LETTERW			72
#define LETTERH			72
#define BOXW			144
#define BOXH			25
#define BOXHGAP         3
#define BOXTEXTOFFSETX	6
#define BOXTEXTOFFSETY	-3
#define MENU_HI_X		58
#define MENU_HI_Y		(LETTERH + 40)
#define MENU_HI_GAP		55
#define MENU_HI_OFF		20
#define FONT_SMALL		20
#define FONT_MEDIUM		26
#define FONT_BIG		34
#define FONT_TINY		14
#define FONT_CLEAN		22
#define FONT_CLEAN_MAX	80
#define ROUNDEL_VEL		20
#define BG_LINE_TOP		85
#define BG_LINE_BOT		415
#define GAME_GAP1		8
#define MAX_WORD_COL	8	//max number of words to display down a 3, 4, 5 or 6 word column
#define MAX_WORD_ROW	4	//4 across screen, 3,4,5,6 for 6, or 5,6,7,8 for 8 letter target word
#define FOUND_WORD_CHR	18	//pixels wide for calculating found word box size

#endif


