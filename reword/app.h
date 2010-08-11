//app.h
 
#ifndef APP_H
#define APP_H
 
#include <string>
#include "error.h"
 
class App : public Error
{
public:
	App();
	~App();
	
	bool 		init();
	bool 		go(void);	//main game loop

private:

	bool		_init;
};

#endif

