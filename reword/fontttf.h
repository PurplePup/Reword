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
	FontTTF(const std::string &fileName, int size, const std::string &desc = "");
	~FontTTF();

	bool load(std::string fontName, int size, const std::string &desc = "");
    bool convertToFastTexture(Screen *s);

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
    Surface * make_surface(const char *textstr, const SDL_Color &textColour, bool bShadow = false);

	void setShadowColour(SDL_Color &c);
	Rect calc_text_metrics(const char *textstr, bool bShadow = false, int xOffset=0, int yOffset=0) const;	//slow as it needs to render internally first
	int calc_text_length(const char *textstr, bool bShadow = false) const;	//slow as it needs to render internally first

    const int size() const { return _size; }        //font size, quick helper fn
    const int height() const { return _height; }    //font height in pixels, quick helper fn
    const std::string & filename() const { return _filename; }  //debug helper
    const std::string & description() const { return _description; }

protected:
	void cleanUp();

private:
	TTF_Font *	_font;
	int			_size;
	bool		_init;
	SDL_Color	_fontColour;
	SDL_Color	_shadowColour;
	char		_buffer[400];	//general purpose char buffer (for number formatting etc)
	int 		_height;
	std::string _filename;
	std::string _description;

	std::unique_ptr<Texture> _fastTex;
	struct SFastWidths
	{
	    int start;
	    int width;
	};
	std::vector<SFastWidths> _fastWidths;
    bool _bFastTex;
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
