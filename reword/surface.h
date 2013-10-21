//surface.h

#if !defined _SURFACE_H
#define _SURFACE_H

#include <SDL.h>
#include <SDL_surface.h>

#include <string>

//#include "utils.h"

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

	bool create(Uint32 w, Uint32 h, int iAlpha = -1);	//create a surface of specific size
    bool load(const std::string &fileName);
	void copy(Surface &s);

    void setTransparentColour(SDL_Color cAlphaKey);
    void setTransparentColour();    //use first pixel
    void setAlphaTransparency(Uint8 iAlpha);

	//accessors
	Uint32 width() const;
	Uint32 height() const;
	SDL_Surface *surface(void) const;
	SDL_PixelFormat* format(void) const;


protected:
	void cleanUp();
	bool initSurface(int iAlpha);
	void setSurface(SDL_Surface *s);

protected:
	SDL_Surface *_surface;

};

#endif //_SURFACE_H

