#ifndef RESOURCE_H
#define RESOURCE_H


#include "singleton.h"

class Resource : public Singleton<Resource>{
private:
    Resource();
    ~Resource();
    Resource(const Resource& other);
    Resource& operator=(const Resource& other);
public:
    friend class FriendClass;

};



#endif // RESOURCE_H
