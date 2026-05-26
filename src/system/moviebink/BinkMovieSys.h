#pragma once
#include "movie/MovieSys.h"
#include "moviebink/BinkMovieImpl.h"
#include "os/CritSec.h"
#include <list>

class BinkMovieSys : public MovieSys {
public:
    BinkMovieSys();
    virtual ~BinkMovieSys();
    virtual void Init();
    virtual void Terminate();
    virtual void Validate() {}
    virtual MovieImpl *CreateMovieImpl() { return new BinkMovieImpl(); }

    void PlatformInit();
    void PlatformStoreCache(void *, unsigned int);
    bool GetUnkC() const { return unkc; }
    int GetUnk10() const { return unk10; }
    int Core0() const { return mBinkCore0; }
    int Core1() const { return mBinkCore1; }

private:
    static DataNode OnMovieSetTrack(DataArray *);

    CriticalSection *mCritSec; // 0x8
    bool unkc; // 0xc
    int unk10; // 0x10 - total num frames?
    int mBinkCore0; // 0x14
    int mBinkCore1; // 0x18
    int mTrack; // 0x1c
    std::list<BinkMovieImpl *> unk20; // 0x20
};

extern BinkMovieSys &TheBinkMovieSys;
