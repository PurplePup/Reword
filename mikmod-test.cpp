#include <mikmod.h>
#include <ios>
#include <iostream>

main()
{
	std::cout << "initialising...\n";
	
	/* register all the drivers */
	MikMod_RegisterAllDrivers();

	/* initialize the library */
	MikMod_Init("");

	/* we could play some sound here... */
	std::cout << "loading...\n";

	MODULE *_modMusic;
    _modMusic = Player_Load("./reword/data/sounds/cascade.mod", 64, 0);
    if(!_modMusic) {
        return false;
    }
    MikMod_EnableOutput();

	std::cout << "start playing...\n";
    Player_Start(_modMusic);

	while (Player_Active())
		MikMod_Update();

	Player_Stop();

	/* give up */
	MikMod_Exit();
}
//g++ -o test test.cpp `libmikmod-config --cflags` `libmikmod-config --libs`

