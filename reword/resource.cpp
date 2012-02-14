#include "resource.h"

Resource::Resource()
{
    //ctor
}

Resource::~Resource()
{
    //dtor
}

Resource::Resource(const Resource& )
{
    //copy ctor
}

Resource& Resource::operator=(const Resource& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
