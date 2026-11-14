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
    bool IsAsync() const { return mAsyncBink; }
    CriticalSection *GetCS() { return mCritSec; }
    int GetNumFramebuffers() const { return mNumFramebuffers; }
    int GetBinkCore0() const { return mBinkCore0; }
    int GetBinkCore1() const { return mBinkCore1; }
    int GetForceTrack() const { return mForceTrack; }
    void SetForceTrack(int track) { mForceTrack = track; }
    void MovieOpened(BinkMovieImpl *movie) { mOpenMovies.push_back(movie); }
    void MovieClosed(BinkMovieImpl *movie) { mOpenMovies.remove(movie); }

private:
    static DataNode OnMovieSetTrack(DataArray *);

    CriticalSection *mCritSec; // 0x8
    bool mAsyncBink; // 0xc
    int mNumFramebuffers; // 0x10
    int mBinkCore0; // 0x14
    int mBinkCore1; // 0x18
    int mForceTrack; // 0x1c
    std::list<BinkMovieImpl *> mOpenMovies; // 0x20
};

extern BinkMovieSys &TheBinkMovieSys;
