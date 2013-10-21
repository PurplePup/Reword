//utils.h

#if !defined _UTILS_H
#define _UTILS_H

#include <SDL.h>
#include "random.h"
#include "surface.h"


struct Point : public SDL_Point
{
public:
	Point() { x=y=0; }
	Point(int newx, int newy) { x = newx; y = newy; }
	Point add(const Point &pt) { return Point(x+pt.x, y+pt.y); }
	Point sub(const Point &pt) { return Point(x-pt.x, y-pt.y); }
	Point mul(int i) { return Point(x*i, y*i); }
	Point div(int i) { return Point(x/i, y/i); }
	bool eq(const Point &pt) const { return x==pt.x && y==pt.y; }
};

struct Rect //: public SDL_Rect
{
public:
	Point _min, _max;

	Rect() : _min(0, 0), _max(0, 0) {}
	Rect(Point pt1, Point pt2) : _min(pt1), _max(pt2) {}
	Rect(int x1, int y1, int x2, int y2) : _min(x1, y1), _max(x2, y2) {}
	Rect(SDL_Rect r) : _min(r.x, r.y), _max(r.x+r.w, r.y+r.h) {}
	SDL_Rect toSDL() const { SDL_Rect r = { _min.x, _min.y, dx(), dy() }; return r; }
	//SDL_Rect operator() (const Rect& r) { return r.toSDL(); }

	int dx() const { return _max.x-_min.x; }
	int dy() const { return _max.y-_min.y; }
	int top() const { return _min.y; }
	int bottom() const { return _max.y; }
	int left() const { return _min.x; }
	int right() const { return _max.x; }
	int height() const { return _max.y - _min.y; }
	int width() const { return _max.x - _min.x; }
	Rect addpt(Point pt) { return Rect(_min.add(pt), _max.add(pt)); }
	Rect subpt(Point pt) { return Rect(_min.sub(pt), _max.sub(pt)); }
	Rect inset(int n) { return Rect(_min.add(Point(n, n)), _max.sub(Point(n, n))); }
	bool contains(const Point &pt) const {
		return pt.x>=_min.x && pt.x<_max.x && pt.y>=_min.y && pt.y<_max.y;
	}
};


class RandInt
{
public:
	RandInt();
	void setSeed(unsigned int seed);
	int random(int limit);
	int operator() (int limit);

	CRandom	m_rnd;					//random number generator class
};


static RandInt g_randInt;

//namespace ppg 	//pp game functions
//{
class ppg
{
public:
	static void pushSDL_Event(int code, void *data1 = nullptr, void *data2 = nullptr);
    static void pushSDL_EventKey(int key);

   	//public draw functions
 //   void drawRect(const Rect& r, const SDL_Color& c);
 //   void drawRect(const SDL_Rect& r, const SDL_Color& c);
 //   void drawRect(int x, int y, int w, int h, const SDL_Color& c);
    static void drawSolidRect(Surface* s, int x, int y, int w, int h, const SDL_Color& c);
    static void drawSolidRectA(Surface* s, int x, int y, int w, int h, const SDL_Color& c, int iAlpha);
	static void putPixel(Surface* s, int x, int y, Uint32 colour);

	static void blit_surface(SDL_Surface* source, SDL_Rect* srcRect, SDL_Surface* dest, int destX, int destY);
	//void blit_surface(SDL_Surface* source, SDL_Rect* srcRect, int destX, int destY );

//    template<typename T> struct Deleter
//    {
//        void operator()(T *p)
//        {
//            delete p;
//        }
//    };
};

//}


#endif //_UTILS_H
