#pragma once
#include "movie/MovieImpl_p.h"

class MovieSys {
public:
    MovieSys();
    virtual ~MovieSys();
    virtual void Init();
    virtual void Terminate();
    virtual void Validate() {}
    virtual MovieImpl *CreateMovieImpl();

    bool IsInitialized() { return mInitialized; }

private:
    bool mInitialized; // 0x4
};

extern MovieSys &TheMovieSys;
