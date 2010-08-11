===============================================================================
                    /---\  /---\  /---\  /---\  /---\  /---\
                   (  R  )(  E  )(  W  )(  O  )(  R  )(  D  )
                    \---/  \---/  \---/  \---/  \---/  \---/
===============================================================================

By Al McLuckie
Any comments/suggestions to al-at-purplepup-dot-org


Synopsis:
=========
REWORD is a simple word game where you are given a jumbled 6 letter word and must 
find at least one valid 6 letter word from it to progress to the next level. 
Smaller 3, 4 and 5 letter words can be found to add to your score. 

New game modes added (as of v0.4) for Speed6 and TimeTrial.

Speed6 is similar to classic Reword, but dispenses with having to find any further 
3, 4 and 5 letter words once a 6 is found.

TimeTrial is again similar, but you are given a limited time in which to get as 
many 6 letter words as possible.

There are currently around 7400 words of which 2800 are 6 letter words.

License:
========
See the file 'Copying' for License.

Installing:
===========
Just unpack the reword .zip directory to your GP2X memory card and run the
reword.gpe batch file, (R) icon, which will launch the reword executable file.
If you need to chain back to a different menu program, make changes in the
gpe batch file.

Upgrading:
If you have a high score file from a previous version it can be placed in the root 
of the data directory (where it was located before) and will be picked up.
As of v0.4, the score file will be converted to the new format incorporating 
speed6 and TimeTrial scores.

Instructions:
=============
Play instructions are given in-game.

Information:
============
There is also a supporting program 'rewordlist' that was used to create the dictionary 
in the format required by REWORD. It can be used to create your own dictionary if you 
want. The source code and instructions are released with the REWORD source code. 

Version History:
================
0.4		18.03.2008	Updates:
		Added game modes - "reword" "speed6" and "time trial"
		Added longer dictionary definitions with scrolling
		Added in-game popup menu (/w warning pinger) for quick exit/save/move-on
		New format score file incl. speed6 & TimeTrial (converts old format)
		Added bonuses for fastest times in Speed6 and TimeTrial
		Added some sounds and animation.
		Changed graphics to be rgb565 for 16 bit display (better gradients)
		Added inclusion/exclusion files to rewordlist dictionary util

0.3.1	08.06.2007	Updates:
		Fixed (I think) rogue letters flying off screen on shuffle
		Fixed memory leak in letter Roundels class destructor
		Fixed pressing Y in pause mode would end game
		Speeded up menus fractionally (ok Jozef?)
		Changed stick+L+R exit to go to hiscore if needed

0.3	30.05.2007	Rewrite:
		Complete rewrite in C++ with sprite and animation classes etc
		Now includes word difficulty settings and (many) word definitions loaded into wordlist
		Word list has ~2900 six letter words.
		Animated all movement aspects of the game to give a more 'arcade' feel to it.
		Added mod music file to menus ( cascade.mod from http://modarchive.org/ no author given)
		Made all sounds 22050Hz for better compatibility
		Restructured data directory. High score file still goes in data dir
		TODO:	improve sprite animation and movement classes
				allow music in game (if wanted) with independent volume control
				etc.

0.2	31.10.2006	Updates:
		Corrected scoring to always place higher scores	first then #words within score.
		Changed shade of orange for medium difficulty to stand out from red better.
		Used REWORDLIST to up the number of 6 letter words to ~1700 by increasing 
			all 3, 4, 5 and 6 letter words cutoff from 7 to 8 
		Reduce wait on BONUS/BADLUCK screen to 1.5 sec from 2 (annoying).
		Lighten letter roundel colour to see letters better. especially for different 
			viewing angles on gp2x.
		Dictionary mods to exclude more bad words and include missing ones.
		Fix resource leak - not releasing a surface
		Fix countdown bug in countdown_callback() when < 0 (in easy mode)
		Fixed launch script exit back to menu properly and 'sync' after exit from game.
		Added loading splash so don't need to load ttf before showing a message. 
			So reduce time showing black screen before 'loading' message can be displayed.
		Improved random seed calculation.
		Change hiscore initials to difficulty level colour and allow
			other hiscore levels to be shown using joy left/right.

0.1	06.09.2006	First version

