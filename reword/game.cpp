////////////////////////////////////////////////////////////////////
/*

File:			game.cpp

Class impl:		Game

Description:	A control class for the whole game. Some initialisation, then
				each screen is created and called from here, passing input and
				events as they happen.

Author:			Al McLuckie (al-at-purplepup-dot-org)
				Based on framework by Dave Parker drparker@freenet.co.uk

Date:			06 April 2007

History:		Version	Date		Change
				-------	----------	--------------------------------
				0.5		16.05.2008	Added mouse (touch screen) support
				0.7		02.01.17	Moved to SDL2

Licence:		This program is free software; you can redistribute it and/or modify
				it under the terms of the GNU General Public License as published by
				the Free Software Foundation; either version 2 of the License, or
				(at your option) any later version.

				This software is distributed in the hope that it will be useful,
				but WITHOUT ANY WARRANTY; without even the implied warranty of
				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
				GNU General Public License for more details.

				You should have received a copy of the GNU General Public License
				along with this program; if not, write to the Free Software
				Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
////////////////////////////////////////////////////////////////////

#include "global.h"

#include <SDL.h>
#include <SDL_image.h>	//for IMG_ functions
//#include <SDL_ttf.h>	//for TTF_ functions
#ifdef GP2X
#include <SDL_gp2x.h>
#endif

#include "screen.h"
#include "game.h"
#include "gamedata.h"	//for colour in fps etc
#include "global.h"
#include "platform.h"

#include "audio.h"
#include "framerate.h"
#include "locator.h"

#include "playmenu.h"
#include "playmainmenu.h"
#include "playmodemenu.h"
#include "playdiff.h"
#include "playhigh.h"
#include "playinst.h"
#include "playoptions.h"
#include "playgame.h"
#include "singleton.h"

#include <cassert>
#include <sys/stat.h>	//to detect touch screen

#ifndef WIN32
//added for frame buffer vsync
#include <linux/fb.h>
#include <fcntl.h>
#include <linux/mman.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#endif

#define MAX_FRAME_RATE 60


Game::Game() :
	_init(false),
	_screen(nullptr), _input(nullptr), _audio(nullptr), _gd(nullptr)
{
}

Game::~Game()
{
	delete _gd;

	//now unload the SDL stuff
	delete _screen;
	delete _input;
	delete _audio;

	if (_init)
	{
#if (defined(GP2X) || defined(PANDORA))
        SDL_FreeCursor(hiddenCursor);
#endif
        IMG_Quit();
		TTF_Quit();
	}
}

void Game::splash()
{
	if (!_screen) return;
	Image img(RES_IMAGES + "splash.png");	//solid black background
	if (img.initDone())
	{
		SDL_Colour c = {0x00,0x00,0x00,0};	//black
		_screen->drawSolidRect(0, 0, Screen::width(), Screen::height(), c);
		//center it - if needed
		_screen->blit(img.texture(), nullptr, (Screen::width()-img.width())/2, (Screen::height()-img.height())/2);
		_screen->update();	//flip
		//sleep(2);	//so we can see it (POSIX)
	}
}

bool Game::loadResources()
{
    bool bErr = false;

    //if (Locator::data()._fntTiny.description() != "Sans tiny")
    //{   //]##DEBUG##
    //    std::cerr << "4 tiny: not == Sans tiny" << std::endl;
    //};


    //register the actual concrete data member holding the resources
	Resource::registerImage(&_images);

    //if (Locator::data()._fntTiny.description() != "Sans tiny")
    //{   //]##DEBUG##
    //    std::cerr << "1 tiny: not == Sans tiny" << std::endl;
    //};


    bErr |= !Resource::image().add("roundel_letters.png", 26);
    bErr |= !Resource::image().add("roundel_kbd_letters.png", 26);

	//SINGLE FRAME BACKGROUNDS & IMAGES
	bErr |= !Resource::image().add("menubg.png");		//solid background (no alpha)
	bErr |= !Resource::image().add("menubg_plain.png");
	bErr |= !Resource::image().add("menu_arcade.png");
	bErr |= !Resource::image().add("menu_reword.png");
	bErr |= !Resource::image().add("menu_speeder.png");
	bErr |= !Resource::image().add("menu_timetrial.png");
	bErr |= !Resource::image().add("scorebar.png");
	bErr |= !Resource::image().add("game_arcade.png");
	bErr |= !Resource::image().add("game_reword.png");
	bErr |= !Resource::image().add("game_speeder.png");
	bErr |= !Resource::image().add("game_timetrial.png");
	bErr |= !Resource::image().add("popup_menu.png");

	//IMAGE TILES (MULTIPLE TILE IMAGES)
    bErr |= !Resource::image().add("cursors.png", 3);
    bErr |= !Resource::image().add("ping_small.png", 6);

	bErr |= !Resource::image().add("boxes.png", 24, ALPHA_COLOUR, 255, Image::TILE_VERT);
	bErr |= !Resource::image().add("scratch.png", 7);

    //BUTTONS
	bErr |= !Resource::image().add("btn_round_scroll_up.png", 5);
	bErr |= !Resource::image().add("btn_round_scroll_down.png", 5);
	bErr |= !Resource::image().add("btn_round_scroll_left.png", 5);
	bErr |= !Resource::image().add("btn_round_scroll_right.png", 5);
	bErr |= !Resource::image().add("btn_round_scroll_up_small.png", 5);
	bErr |= !Resource::image().add("btn_round_scroll_down_small.png", 5);

	bErr |= !Resource::image().add("btn_round_word_shuffle.png", 5);
	bErr |= !Resource::image().add("btn_round_word_try.png", 5);
	bErr |= !Resource::image().add("btn_round_word_totop.png", 5);
	bErr |= !Resource::image().add("btn_round_word_last.png", 5);

    bErr |= !Resource::image().add("btn_square_menu.png", 5);
    bErr |= !Resource::image().add("btn_square_exit.png", 5);
    bErr |= !Resource::image().add("btn_square_next.png", 5);
    bErr |= !Resource::image().add("btn_square_back_small.png", 5);
    bErr |= !Resource::image().add("btn_square_exit_small.png", 5);
    bErr |= !Resource::image().add("btn_square_next_small.png", 5);
    bErr |= !Resource::image().add("btn_square_yes_no_small.png", 9);
    bErr |= !Resource::image().add("btn_square_diff.png", 13);

    bErr |= !Resource::image().add("btn_round_music.png", 9);
    bErr |= !Resource::image().add("btn_round_fx.png", 9);

	bErr |= !Resource::image().add("star.png", 7);

    //sound resources
    bErr |= Locator::audio().addSfx("ping.wav", AUDIO_SFX_PING) == -1;
    bErr |= Locator::audio().addSfx("boing.wav", AUDIO_SFX_NOTINDICT) == -1;
    bErr |= Locator::audio().addSfx("beepold.wav", AUDIO_SFX_ALREADYDONE) == -1;
    bErr |= Locator::audio().addSfx("honk.wav", AUDIO_SFX_NOT6) == -1;
    bErr |= Locator::audio().addSfx("binkbink.wav", AUDIO_SFX_FOUND6) == -1;
    bErr |= Locator::audio().addSfx("blipper.wav", AUDIO_SFX_FOUNDNON6) == -1;
    bErr |= Locator::audio().addSfx("fanfare.wav", AUDIO_SFX_ALLFOUND) == -1;
    bErr |= Locator::audio().addSfx("woosh2.wav", AUDIO_SFX_JUMBLE) == -1;
    bErr |= Locator::audio().addSfx("blipper.wav", AUDIO_SFX_ROUNDEL) == -1;
    bErr |= Locator::audio().addSfx("blipper.wav", AUDIO_SFX_CONTROL) == -1;

    return !bErr;   //true = success
}

bool Game::init(GameOptions &options)
{
	std::cout << "Using hardware/keyboard profile : " << DEBUG_HW_NAME << std::endl;

	//sanity check
	assert(false==_init);	//shouldn't be called once initialised successfully

    Uint32 init_flags = SDL_INIT_EVERYTHING;
    if (!options._bSound)
    {
        std::cout << "Audio disabled" << std::endl;
        init_flags &= ~SDL_INIT_AUDIO;
    }

	//Init SDL but if anything borks, just exit
	if ( SDL_Init(init_flags) < 0 )
	{
		setLastError("Unable to init SDL");
		return false;
	}

	atexit(SDL_Quit);	//auto cleanup, just in case

    //Set texture filtering to linear
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
    {
        std::cout << "Warning: Linear texture filtering not enabled!";
    }

    if ((init_flags & SDL_INIT_AUDIO) == 0)
        Locator::initAudio();   //NullAudio in logs window
    else
        Locator::registerAudio(_audio = new Audio());
    IAudio &audio = Locator::audio();
    audio.setup(options._bDefaultSfxOn, options._bDefaultMusicOn, options._defaultMusicDir, options._bMute);

	_screen  = new Screen(SCREEN_WIDTH, SCREEN_HEIGHT, "REWORD");
	if (!_screen->initDone())
	{
		setLastError(_screen->lastError());
		return false;
	}
	Locator::registerScreen(_screen);

    //Initialize PNG loading
    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) )
    {
        std::string s = "SDL_image could not initialize! SDL_image Error: ";
        s += IMG_GetError();
        setLastError(s, false);
        return false;
    }

#if (defined(GP2X) || defined(PANDORA))
    uint8_t hiddenCursorData = 0;
    hiddenCursor = SDL_CreateCursor(&hiddenCursorData, &hiddenCursorData, 8, 1, 0, 0);
    /* On the OpenPandora we need to work around an SDL assumption that
       returns relative mouse coordinates when you get to the screen
       edges using the touchscreen and a custom surface for the cursor.
       The workaround is to set a blank SDL cursor and NOT disable it
       (Hackish I know).

       The root issues lies in the Windows Manager GRAB code in SDL.
       That is why the issue is not seen on framebuffer devices like the
       GP2X (there is no X window manager ;) ).
    */
    SDL_ShowCursor(SDL_ENABLE);
    SDL_SetCursor(hiddenCursor);
    //SDL_ShowCursor (0);	//hide - place here after video init else stays on screen on GP2X
#endif
	splash();				//show splash png not text

	//Initialize SDL_ttf
	if( TTF_Init() == -1 )
	{
		setLastError("Unable to init fonts");
		return false;
	}

	_input = new Input();
	Locator::registerInput(_input);
	_input->init();

#if ((defined(GP2X) || defined(PANDORA)))
	//SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
//	joystick = SDL_JoystickOpen(0);
#endif

	//load all game data (images, fonts, etc, etc)
	_gd = new GameData();
	_gd->setOptions(options);
	_gd->_current_w = _screen->width();
	_gd->_current_h = _screen->height();
	_gd->init(); //load main resources
    Locator::registerData(_gd);

#if defined(_USE_OGG)
	//load mp3/ogg menu music
	std::cout << "Loading menu.ogg music" << std::endl;
	_gd->_musicMenu = Mix_LoadMUS(std::string(RES_SOUNDS + "menu.ogg").c_str());
#endif

#if defined(GP2X)
	std::cout << "Using GP2X touchscreen device" << std::endl;
	struct stat stFileInfo;
	if(stat("/dev/touchscreen/wm97xx",&stFileInfo) == 0)
		_gd->_bTouch = true;	//F200 touchscreen available
#else
	std::cout << "Using standard touchscreen device" << std::endl;
	_gd->_bTouch = true;	//Windows, Pandora etc, uses mouse
#endif

    //finally load image and sound resources
	if ( !loadResources() )
	{
		setLastError("Unable to load resources");
		return false;
	}


    if (Locator::data()._fntTiny.description() != "Sans tiny")
    {   //]##DEBUG##
        std::cerr << "6 tiny: not == Sans tiny" << std::endl;
    };


	return (_init = (_gd && _gd->isLoaded()));
}

//factory fn to run the game,
//and manage the state transitions between screens etc
bool Game::run(void)
{
	assert(_init);  //should have called Init() by now

	//call different play classes
	IPlay *p = 0;
	bool b = true;
	while (b &&_gd->_state != ST_EXIT)
	{
		switch (_gd->_state)
		{
		case ST_MENU:		p = new PlayMainMenu(*_gd);break;

		case ST_MODE:		p = new PlayModeMenu(*_gd);break;

		case ST_RESUME:
		case ST_GAME:		p = new PlayGame(*_gd);break;

		case ST_DIFF:		p = new PlayDiff(*_gd);break;

		case ST_HIGH:
		case ST_HIGHEDIT:	p = new PlayHigh(*_gd);break;

		case ST_INST:		p = new PlayInst(*_gd);break;

		case ST_OPTN:		p = new PlayOptions(*_gd);break;

		default: return false;
		}

		b = play(p);	//do the biz!
		delete p;
	}

	std::cout << "Exiting Reword - all ok" << std::endl;

	return b;
}


#define MAXIMUM_FRAME_RATE 60
//#define MINIMUM_FRAME_RATE 15
//#define UPDATE_INTERVAL (1.0 / MAXIMUM_FRAME_RATE)	//0.0166666
//#define MAX_CYCLES_PER_FRAME (MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE)	//4


bool Game::play(IPlay *p)
{
    if (nullptr == p) return false;    //invalid IPlay object

	bool bCap = true;
#ifdef GP2X
	int touchX(0), touchY(0);
#endif

/*	//tinkering... Fixed interval time-based animation variables
	double lastFrameTime = SDL_GetTicks();//0.0;
	double cyclesLeftOver = 0.0;
	double currentTime;
	double updateIterations;
*/
	//vars used here, mainly for events and fps
	SDL_Event event;

#ifndef WIN32
//	int fbdev = open("/dev/fb0", O_RDONLY);
//  //void *buffer =
//      mmap(0, SCREEN_WIDTH*SCREEN_HEIGHT*2, PROT_WRITE, MAP_SHARED, fbdev, 0);
#endif
	// Initialise play/level specific stuff
	p->init(_input, _screen);

	Framerate fr;
	fr.init(MAXIMUM_FRAME_RATE);

    // Main loop
    while(p->running())
    {
		fr.setSpeedFactor();

/*
		//tinkering...
		currentTime = SDL_GetTicks();
		updateIterations = ((currentTime - lastFrameTime) + cyclesLeftOver);
		if (updateIterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {	//4*0.01666 = 0.0666666666
			updateIterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
		}
		while (updateIterations > UPDATE_INTERVAL)
		{
			updateIterations -= UPDATE_INTERVAL;
			// Update game state a variable number of times
*/
			// Do work/think stuff
			//_gd->_fact = fr.speedFactor();
			p->work(_input, fr.speedFactor());

			_gd->_effects.work();

			// Handle SDL events
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					// Handle keys
					// a - a
					// b - b
					// x - x
					// y - y
					// l - l
					// r - r
					// + - vol+
					// - - vol-
					// Return - select
					// Start  - space
					// Cursor keys - joystick

					case SDL_KEYDOWN:
					{
						ppkey::eButtonType b = _input->translate(event.key.keysym.sym);
						if (b != ppkey::NUMBER_OF_BUTTONS)
						{
							_input->down(b);
							p->button(_input, b);
						}
					}
					break;

					case SDL_KEYUP:
					{
						// If escape is pressed...
						if (event.key.keysym.sym == SDLK_ESCAPE)
						{
							if (_gd->_state == ST_MENU)
                            {
                                _gd->_state = ST_EXIT;
                                return true; //exit main menu & game
                            }
							_gd->_state = ST_MENU;	//back to menu screen
							p->quit();				//sets _running false (if not overridden)
							break;
						}

#ifdef _DEBUG
                        //if 'f' key pressed, toggle capped framerate
						if (event.key.keysym.sym == SDLK_f)	bCap = !bCap;

						// if 'd' key pressed, toggle debug display
						if (event.key.keysym.sym == SDLK_d)
                        {
                            p->toggleDbgDisplay();
                        }
#endif

						ppkey::eButtonType b = _input->translate(event.key.keysym.sym);
						if (b != ppkey::NUMBER_OF_BUTTONS)
						{
							_input->up(b);
							p->button(_input, b);
						}

						if (ppkey::VOLUP == b)
							Locator::audio().volumeUp();
						else
							if (ppkey::VOLDOWN == b)
								Locator::audio().volumeDown();

					}
					break;

					// Handle GP2X button events

					case SDL_JOYBUTTONUP:
					{
						ppkey::eButtonType b = static_cast<ppkey::eButtonType>(event.jbutton.button);
						_input->up(b);
						p->button(_input, b);

						if (ppkey::VOLUP == b)
							Locator::audio().volumeUp();
						else
							if (ppkey::VOLDOWN == b)
								Locator::audio().volumeDown();
					}
					break;

					case SDL_JOYBUTTONDOWN:
					{
						ppkey::eButtonType b = static_cast<ppkey::eButtonType>(event.jbutton.button);
						_input->down(b);
						p->button(_input, b);
					}
					break;

                    //'touch' denotes a press of the pointing device, and can be acted on immediately or
                    //the called code can wait for a 'tap' of the pointing device releasing.
					case SDL_MOUSEBUTTONDOWN:
					{
						p->touch(Point(event.button.x, event.button.y));
					}
					break;

                    //tap denotes a release of the pointing device after a touch. The called code can act
                    //on this or check the initial touch position to see if there was a slide away to cancel.
					case SDL_MOUSEBUTTONUP:
					{
						p->tap(Point(event.button.x, event.button.y));
					}
					break;

					case SDL_QUIT:
						return true;	//valid exit

					default:
						//ok, here we handle all *our* events, user defined or pass on
						//any others in case it's handled elsewhere (ie, in game level)
						if (event.type == SDL_USEREVENT)
						{
							switch (event.user.code)
							{
                            case USER_EV_START_NEXT_TRACK:
                                Locator::audio().startNextTrack();
                                break;
                            case USER_EV_START_MENU_MUSIC:
                                //play menu music if enabled and not already playing
                                if (Locator::audio().musicEnabled() && !Locator::audio().isActuallyPlayingMusic())
                                    Mix_PlayMusic(_gd->_musicMenu, -1);	//play 'forever' - or until game starts
                                break;
/*							case USER_EV_SAVE_STATE:
								_gd->saveQuickState();
								break;*/
							default:
								p->handleEvent(event);	//let the play impl handle other user events
								break;
							}
						}
						else
							p->handleEvent(event);	//let the play impl handle other events
				}
			}

//#ifdef GP2X
//			if (SDL_GP2X_Touchpad(&touchX, &touchY))
//			{
//				p->touch(Point(touchX, touchY));
//			}
//#endif

/*
		//tinkering...
		}	//while (updateIterations > UPDATE_INTERVAL)
		cyclesLeftOver = updateIterations;
		lastFrameTime = currentTime;
*/
		_screen->lock();
		_screen->clear();

		p->render(_screen);	//screen render

        //render short term effects over any other renders
        _gd->_effects.draw(_screen);

#ifdef _DEBUG	//overlay the framerate and any other debug info required
		_gd->_fntSmall.put_number(_screen,0,60,fr.fps(),"%d", BLACK_COLOUR);
#endif


#ifndef WIN32
		//be sure to pass argument value 0 or it will not work.
		//Currently FBIO_WAITFORVSYNC is not defined in <linux/fb.h>,
		//although this is in the process of modification.
		//For now, define in the following manner:
		#ifndef FBIO_WAITFORVSYNC
		  #define FBIO_WAITFORVSYNC _IOW('F', 0x20, __u32)
		#endif
//		int arg = 0;
//		ioctl(fbdev, FBIO_WAITFORVSYNC, &arg);
#endif

		_screen->unlock();
		_screen->update();

		if (bCap) fr.capFrames();

//#ifdef _USE_MIKMOD
//		Locator::audio().modUpdate();
//#endif

    }	//while p->running()

	//on exiting the screen, reset all keys repeat rates to none.
	//Its up to the next screen (play class) to set as required on entry.
	_input->clearRepeat();

//	std::cout << "exit game loop ok" << std::endl;		//##DEBUG
	return true;
}


