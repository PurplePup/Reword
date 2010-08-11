//singleton.h

#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <memory>

/*
template<class T>
class Singleton
{
public:
    static T *instance()			// Create an instance of the object
//    {
//	    if (_instance.get() == 0)
//	      _instance.reset(new Singleton);
//	    return *_instance;
//	}
	;
	
    static T &getRef()			// Get a reference to the object
//	{
//		return *_instance;
//	}
	;
	
    static T *getPtr()			// Get a pointer to the object
//	{
//		return _instance.get();
//	}
    ;
 
protected:
    Singleton();   // constructor NOT public
 
private:
    static std::auto_ptr<T> _instance;
};


template <class T>
  	std::auto_ptr<T> Singleton<T>::_instance(0);    //   definition


// Create an instance of the object, or return the existing ptr
template<class T> T *Singleton<T>::instance()
{
    if (_instance.get() == 0)
      _instance.reset(new Singleton);
    return *_instance;
}

// return reference to the object
template<class T> T &Singleton<T>::getRef()
{
	return *_instance;
}

// return pointer to the object
template<class T> T *Singleton<T>::getPtr()
{
	return _instance.get();
}
*/

#endif //_SINGLETON_H


/*
A Singleton should not have public constructors as in your above class.
If this is a singleton wrapper class, then IMHO, it's not practical to
create a singleton that requires constructor arguments.
You're singleton should also have copy constructor and assignment
operator private or protected.
It should also have the destructor private or protected, however some
compilers like VC++ have problems compiling class with private or
protected destructors.

The following Singleton wrapper class has protected destructor. It
should compile on any compliant compiler, and it also compiles on VC++
6.0
Using this wrapper class you can make any class a singleton by either
inheriting from the Singleton wrapper class, or by calling
Singleton::Instance with a template type.

#include <stdlib.h>

template<typename T>
class Singleton
{
protected:
Singleton(){}
~Singleton(){}
Singleton(const Singleton&);
Singleton& operator=(const Singleton&);
public:
class FriendClass
{
public:
FriendClass():m_MyClass(new T()){}
~FriendClass(){delete m_MyClass;}
T* m_MyClass;
};
static T& Instance() {
static FriendClass Instance;
return *Instance.m_MyClass;
}
};

class Widget {
private:
Widget(){}
~Widget(){}
Widget& operator=(const Widget&);
Widget(const Widget&);
public:
friend class Singleton<Widget>::FriendClass;
int m_i;
};


class foo : public Singleton<foo>{
private:
foo(){}
~foo(){}
foo& operator=(const foo&);
foo(const foo&);
public:
friend class FriendClass;
int m_i;
};


int main(int argc, char* argv[])
{
Widget& MyWidget = Singleton<Widget>::Instance();
foo& Myfoo = foo::Instance();

system("pause");
return 0;
}
*/

