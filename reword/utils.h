//utils.h

#ifndef _UTILS_H
#define _UTILS_H

#include "SDL.h"
#include "random.h"


class Point
{
public:
	int _x, _y;
	Point() : _x(0), _y(0) {}
	Point(int x, int y) : _x(x), _y(y) {}
	Point add(const Point &pt) { return Point(_x+pt._x, _y+pt._y); }
	Point sub(const Point &pt) { return Point(_x-pt._x, _y-pt._y); }
	Point mul(int i) { return Point(_x*i, _y*i); }
	Point div(int i) { return Point(_x/i, _y/i); }
	bool eq(const Point &pt) const { return _x==pt._x && _y==pt._y; }
};

class Rect {
public:
	Point _min, _max;

	Rect() : _min(0, 0), _max(0, 0) {}
	Rect(Point pt1, Point pt2) : _min(pt1), _max(pt2) {}
	Rect(int x1, int y1, int x2, int y2) : _min(x1, y1), _max(x2, y2) {}
	Rect(SDL_Rect r) : _min(r.x, r.y), _max(r.x+r.w, r.y+r.h) {}
	SDL_Rect toSDL() { SDL_Rect r = { _min._x, _min._y, dx(), dy() }; return r; }

	int dx() { return _max._x-_min._x; }
	int dy() { return _max._y-_min._y; }
	int top() const { return _min._y; }
	int bottom() const { return _max._y; }
	int left() const { return _min._x; }
	int right() const { return _max._x; }
	int height() const { return _max._y - _min._y; }
	int width() const { return _max._x - _min._x; }
	Rect addpt(Point pt) { return Rect(_min.add(pt), _max.add(pt)); }
	Rect subpt(Point pt) { return Rect(_min.sub(pt), _max.sub(pt)); }
	Rect inset(int n) { return Rect(_min.add(Point(n, n)), _max.sub(Point(n, n))); }
	bool contains(const Point &pt) const {
		return pt._x>=_min._x && pt._x<_max._x && pt._y>=_min._y && pt._y<_max._y;
	}
};

class RandInt
{
public:
	RandInt() {
//SDL.h may not have been included
#ifdef _SDL_H
#ifdef WIN32
#pragma message("words.h: Using SDL_GetTicks() to seed random\n")
#endif
		setSeed(SDL_GetTicks());
#else
#ifdef WIN32
#pragma message("words.h: Using ctime to seed random\n")
#endif
		m_rnd.Randomize();	//uses ctime to seed
#endif
	}	//incase setSeed not called
	void setSeed(unsigned int seed) {
		m_rnd.SetRandomSeed(seed);
		m_rnd.Randomize();
	}
	int random(int limit)
	{
		return (int)m_rnd.Random(limit);
	}
	int operator() (int limit)	//for sorting calls
	{
		return (int)m_rnd.Random(limit);
	}
	CRandom	m_rnd;					//random number generator class
};

static RandInt g_randInt;

namespace pp_g 	//pp game functions
{

	void pushSDL_Event(int code, void *data1 = 0, void *data2 = 0);
    void pushSDL_EventKey(int key);
}


#endif //_UTILS_H
