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

    void setObjectId(int id) { _objectId = id; }
    int getObjectId() { return _objectId; }

	bool create(unsigned int w, unsigned int h, int iAlpha = -1);	//create a surface of specific size
	void copy(Surface &s);

	//accessors
	int width() const;
	int height() const;
	SDL_Surface *surface(void) const;
	SDL_PixelFormat* format(void) const;

	//public draw functions
 //   void drawRect(const Rect& r, const SDL_Color& c);
 //   void drawRect(const SDL_Rect& r, const SDL_Color& c);
 //   void drawRect(int x, int y, int w, int h, const SDL_Color& c);
    void drawSolidRect(int x, int y, int w, int h, const SDL_Color& c);
    void drawSolidRectA(int x, int y, int w, int h, const SDL_Color& c, int iAlpha);
	void putPixel(int x, int y, Uint32 colour);

	static void blit_surface(SDL_Surface* source, SDL_Rect* srcRect, SDL_Surface* dest, int destX, int destY);
	void blit_surface(SDL_Surface* source, SDL_Rect* srcRect, int destX, int destY );

protected:
	void cleanUp();
	bool initSurface(SDL_Surface *newSurface, int iAlpha);
	void setSurface(SDL_Surface *s);

protected:
	SDL_Surface *_surface;
	int _objectId;          //used for event notification in surface/image/imageanim/sprite classes

private:
	SDL_Rect _r;			//temp, used in blit fns

};

#endif //_SURFACE_H

