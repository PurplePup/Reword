//fontttf.h


#if !defined FONTTTF_H
#define FONTTTF_H

#include <SDL.h>
#include <SDL_ttf.h>	//for TTF_ functions
#include <map>
#include <memory>

#include <string>

#include "sprite.h"
#include "surface.h"

class FontTTF
{
public:
	FontTTF();
	FontTTF(std::string fileName, int size);
	~FontTTF();

	bool load(std::string fontName, int size);

    //render font to the screen
	Rect put_text(Screen *s, int x, int y, const char *textstr, const SDL_Color &textColour, bool bShadow = false);
	Rect put_text(Screen *s, int y, const char *textstr, const SDL_Color &textColour, bool bShadow = false);
	Rect put_text_right(Screen *s, int y, int xDelta, const char *textstr, const SDL_Color &textColour, bool bShadow = false);
    Rect put_text_mid(Screen *s, int y, int xMid, const char *textstr, const SDL_Color &textColour, bool bShadow = false);

	Rect put_number(Screen *s, int x, int y, int number, const char *format, const SDL_Color &textColor, bool bShadow = false);
	Rect put_number(Screen *s, int y, int number, const char *format, const SDL_Color &textColor, bool bShadow = false);
	Rect put_number_right(Screen *s, int y, int xDelta, int number, const char *format, const SDL_Color &textColour, bool bShadow = false);
    Rect put_number_mid(Screen *s, int y, int xMid, int number, const char *format, const SDL_Color &textColour, bool bShadow = false);

    //render font to a surface
	Rect put_text(Surface *s, int x, int y, const char *textstr, const SDL_Color &textColour, bool bShadow = false);
	Rect put_text(Surface *s, int y, const char *textstr, const SDL_Color &textColour, bool bShadow = false);
	Rect put_text_right(Surface *s, int y, int xDelta, const char *textstr, const SDL_Color &textColour, bool bShadow = false);
    Rect put_text_mid(Surface *s, int y, int xMid, const char *textstr, const SDL_Color &textColour, bool bShadow = false);

	Rect put_number(Surface *s, int x, int y, int number, const char *format, const SDL_Color &textColor, bool bShadow = false);
	Rect put_number(Surface *s, int y, int number, const char *format, const SDL_Color &textColor, bool bShadow = false);
	Rect put_number_right(Surface *s, int y, int xDelta, int number, const char *format, const SDL_Color &textColour, bool bShadow = false);
    Rect put_number_mid(Surface *s, int y, int xMid, int number, const char *format, const SDL_Color &textColour, bool bShadow = false);

    Texture * make_texture(const char *textstr, bool bShadow = false);

	void setShadowColour(SDL_Color &c);
	Rect calc_text_metrics(const char *textstr, bool bShadow = false, int xOffset=0, int yOffset=0) const;	//slow as it needs to render internally first
	int calc_text_length(const char *textstr, bool bShadow = false) const;	//slow as it needs to render internally first

	int height() const; //helper to quickly return font height in pixels

    //font texture cache
    Uint32 add_cache(const char *textstr, const SDL_Color &textColour, bool bShadow = false);
    void put_cache(Screen *s, Uint32 index, int x, int y);
    void put_cache_right(Screen *s, Uint32 index, int xDelta, int y);

    Surface * make_surface(const char *textstr, const SDL_Color &textColour, bool bShadow = false);

protected:
	void cleanUp();

private:
	TTF_Font *	_font;
	int			_size;
	bool		_init;
	SDL_Color	_shadowColour;
	char		_buffer[400];	//general purpose char buffer (for number formatting etc)
	int 		_height;
};

class FontCache
{
public:
    Uint32 add(FontTTF &ttf, Uint32 index, const char * text, const SDL_Color &textColour, bool bShadow = false);
    Image * get(Uint32 index);

private:
    tImageMap _imageMap;
};

#endif //FONTTTF_H
