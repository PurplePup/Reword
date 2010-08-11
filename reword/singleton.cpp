////////////////////////////////////////////////////////////////////
/*

File:			singleton.cpp

Class impl:		Singleton

Description:	A template class to allow creation of other classes 
				as singletons. Uses a auto_ptr for assured destruction
				Useage:

				//creation
					Singleton<MyClass>::Instance();
				or:
					MyClass *p = Singleton<MyClass>::Instance();
					
				//calling
					Singleton<MyClass>::getPtr()->myFunction();
				or:
					MyClass *p = Singleton<MyClass>::getPtr();
					p->myFunction();

Author:			Al McLuckie (al-at-purplepup-dot-org)

Date:			15 April 2007

Licence:		This program is free software; you can redistribute it and/or modify
				it under the terms of the GNU General Public License as published by
				the Free Software Foundation; either version 2 of the License, or
				(at your option) any later version.

				This software is distributed in the hope that it will be useful,
				but WITHOUT ANY WARRANTY; without even the implied warranty of
				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
				GNU General Public License for more details.

				You should have received a copy of the GNU General Public License
				along with this program; if not, write to the Free Software
				Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
////////////////////////////////////////////////////////////////////


//#include "singleton.h"


//NOTE : DOES NOT WORK IN VC6, MOST OTHER NEWER COMPILERS OK THOUGH


//template <class T>
//  	std::auto_ptr<T> Singleton<T>::_instance(0);    //   definition

/*
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
