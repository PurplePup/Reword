//surface.h

#ifndef _SURFACE_H
#define _SURFACE_H

#include <SDL.h>
#include <SDL_image.h>	//for IMG_ functions

#include "utils.h"

class Surface
{
public:
	Surface();
	Surface(SDL_Surface *s);
	virtual ~Surface();

	Surface & operator=(const Surface &rhs)
	{
		if (this != &rhs)
		{
			this->cleanUp();
			this->_surface = rhs.surface();
			rhs.surface()->refcount++;
		}
		return *this;
	}

	bool create(unsigned int w, unsigned int h, int iAlpha = -1);	//create a surface of specific size
	void copy(Surface &s);

	//accessors
	int width() const;
	int height() const;
	SDL_Surface *surface(void) const;
	SDL_PixelFormat* format(void) const;


protected:
	void cleanUp();
	bool initSurface(SDL_Surface *newSurface, int iAlpha);
	void setSurface(SDL_Surface *s);

protected:
	SDL_Surface *_surface;

};

#endif //_SURFACE_H

