#include "moviebink/BinkMovieSys.h"
#include "bink.h"
#include "movie/MovieSys.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/MemMgr.h"

BinkMovieSys gBinkMovieSys;
BinkMovieSys &TheBinkMovieSys = gBinkMovieSys;

namespace {
    void *RadAlloc(unsigned int size) {
        return MemAlloc(size, __FILE__, 0x28, "Movie", 0x80);
    }
    void RadFree(void *ptr) { MemFree(ptr); }
}

BinkMovieSys::BinkMovieSys() : mCritSec(0), mBinkCore0(-1), mBinkCore1(-1), mTrack(0) {
    unkc = true;
    unk10 = 1;
}

BinkMovieSys::~BinkMovieSys() { RELEASE(mCritSec); }

void BinkMovieSys::Init() {
    bool initial = IsInitialized();
    MovieSys::Init();
    MILO_ASSERT(IsInitialized(), 0x67);
    if (!mCritSec) {
        mCritSec = new CriticalSection();
    }
    CritSecTracker tracker(mCritSec);
    DataArray *cfg = SystemConfig("movie");
    cfg->FindData("bink_core0", mBinkCore0);
    cfg->FindData("bink_core1", mBinkCore1);
    if (!initial) {
        BinkSetMemory(RadAlloc, RadFree);
        PlatformInit();
        if (unkc) {
            MILO_ASSERT_FMT(
                BinkStartAsyncThread(mBinkCore0, nullptr)
                    && BinkStartAsyncThread(mBinkCore1, nullptr),
                "Error starting bink async thread.\n"
            );
        }
    }
    DataRegisterFunc("set_bink_track", OnMovieSetTrack);
}

void BinkMovieSys::Terminate() {
    {
        CritSecTracker tracker(mCritSec);
        // do you not pop from the list after you terminate?
        while (mMovies.size()) {
            mMovies.back()->Terminate();
        }
    }
    RELEASE(mCritSec);
    MovieSys::Terminate();
}

DataNode BinkMovieSys::OnMovieSetTrack(DataArray *a) {
    TheBinkMovieSys.mTrack = a->Int(1);
    return 0;
}
