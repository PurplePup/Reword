#if !defined SCREEN_H
#define SCREEN_H

#include <SDL.h>
#include <SDL_video.h>

#include "error.h"
//#include "surface.h"
#include "utils.h"

class Screen : public Error //, public Surface
{
public:
    // Construct 16 bit colour screen of given size
    Screen ();
    Screen (int w, int h, const std::string &strTitle);
    ~Screen();

    SDL_Texture * texture() { return _texture; }
    SDL_Renderer * renderer() { return _renderer; }

	void lock(void);		// Lock screen
    void unlock(void);		// Unlock screen
    void update(void);		// Update whole screen (flip)

    void clear();
    void drawSolidRect (int x, int y, int w, int h, const SDL_Color& c);
    void drawSolidRectA(int x, int y, int w, int h, const SDL_Color& c, int iAlpha);
    void putPixel(int x, int y, Uint32 colour);
    void blit(SDL_Texture* source, SDL_Rect* srcRect, int destX, int destY);

    // Accessor Methods
	bool initDone() const { return _init; }

	//statics
	static int _width;
	static int _height;
	static int width() { return _width; }
	static int height() { return _height; }

private:
	SDL_Window      * _window;
	SDL_Renderer    * _renderer;
	SDL_Texture     * _texture;

	bool    	    _init;
};

#endif //SCREEN_H

