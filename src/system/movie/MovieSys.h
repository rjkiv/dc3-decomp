#pragma once

#include "MovieImpl.h"
class MovieSys {
public:
    virtual ~MovieSys();
    virtual void Init();
    virtual void Terminate();
    virtual void Validate() {}
    virtual MovieImpl *CreateMovieImpl();

    bool isInitalized;
    MovieSys();
    bool IsInitialized() { return isInitalized; }
};

extern MovieSys &TheMovieSys;
