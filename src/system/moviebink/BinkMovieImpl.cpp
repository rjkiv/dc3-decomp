#include "moviebink/BinkMovieImpl.h"
#include "moviebink/BinkMovieSys.h"
#include "moviebink/BinkMovieLoader.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OSFuncs.h"
#include "os/Platform.h"
#include "os/ThreadCall.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "xdk/XAPILIB.h"

std::vector<BinkMovieImpl *> BinkMovieImpl::sActiveMovies;

#pragma region BinkMovieLoader

BinkMovieLoader::BinkMovieLoader(const FilePath &fp, LoaderPos pos, BinkMovieImpl *impl)
    : Loader(fp, pos), mFile(0), mImpl(impl) {
    mState = &BinkMovieLoader::OpenFile;
}

void BinkMovieLoader::OpenFile() {
    mFile = NewFile(LoaderFile().c_str(), FILE_OPEN_READ);
    if (mFile && !mFile->Fail()) {
        mFile->ReadAsync(mBuffer, sizeof(mBuffer));
        mState = &BinkMovieLoader::LoadFile;
    } else {
        MILO_NOTIFY("Could not load: %s", FileLocalize(LoaderFile().c_str(), nullptr));
        mState = &BinkMovieLoader::DoneLoading;
    }
}

void BinkMovieLoader::DoneLoading() {}

#pragma endregion
#pragma region BinkMovieImpl

#define CHECK_THREAD                                                                     \
    {                                                                                    \
        DWORD cur = GetCurrentThreadId();                                                \
        MILO_ASSERT_FMT(                                                                 \
            mThreadId == cur || (mThreadId == -1 && MainThread()),                       \
            "%s called in the wrong thread (expected %d, cur thread is %d)",             \
            __FUNCTION__,                                                                \
            mThreadId,                                                                   \
            GetCurrentThreadId()                                                         \
        );                                                                               \
    }

BinkMovieImpl::BinkMovieImpl()
    : mLoader(0), mMovieLoader(0), mBink(0), unk18(0), mPreloadBuf(0), unk20(0), unk24(0),
      unk40(0), mWidth(0), mHeight(0), mPaused(0), unkb8(kNoHandle), unkd4(0), unkd5(0),
      mThreadId(gMainThreadID), unke0(0x8000), unke4(0) {
    CHECK_THREAD;
}

BinkMovieImpl::~BinkMovieImpl() {
    CHECK_THREAD;
    End();
}

void BinkMovieImpl::SetWidthHeight(int w, int h) {
    CHECK_THREAD;
    mWidth = w;
    mHeight = h;
}

bool BinkMovieImpl::Ready() const {
    CHECK_THREAD;
    if (mLoader) {
        return mLoader->IsLoaded();
    } else if (mMovieLoader) {
        return mMovieLoader->IsLoaded();
    } else {
        return true;
    }
}

bool BinkMovieImpl::BeginFromFile(
    const char *c1,
    float f2,
    bool b1,
    bool b2,
    bool b3,
    bool b4,
    int i5,
    BinStream *bs,
    LoaderPos pos
) {
    CHECK_THREAD;
    if (TheLoadMgr.GetPlatform() == kPlatformNone) {
        return false;
    } else {
        unkc = c1;
        unk18 = b3;
        if (!PlatformCacheFile(c1)) {
            return false;
        } else {
            unk26 = b2;
            unk27 = b1;
            unkdc = i5;
            unk28 = b4;
            unkb8 = kNoHandle;
            mAspect = 0;
            unkd6 = TheBinkMovieSys.GetUnkC();
            unk50.Reset();
            MILO_ASSERT(!mLoader, 0xD6);
            MILO_ASSERT(!mBink, 0xD7);
            MILO_ASSERT(!mPreloadBuf, 0xD8);
            if (b3) {
                static int _x(MemFindHeap("physical"));
                MemHeapTracker tmp(_x);
                const char *str = unkc.c_str();
                BinStream *bs6 = nullptr;
                if (bs && bs->Cached()) {
                    bs6 = bs;
                }
                mLoader = new FileLoader(str, str, pos, 0, true, true, bs6, "misc");
            } else {
                mMovieLoader = new BinkMovieLoader(unkc.c_str(), kLoadStayBack, this);
            }
            sActiveMovies.push_back(this);
            sActivePending++;
            if (sActivePending > 1 && !b3) {
                MILO_NOTIFY("%s, multiple movies must be preloaded", unkc);
            }
            unkd4 = true;
            return true;
        }
    }
}

bool BinkMovieImpl::BeginFromBuffer(
    void *iBuffer, int iBufSizeBytes, bool b1, float f1, bool b2, bool b3, bool b4, int i1
) {
    CHECK_THREAD;
    MILO_ASSERT(iBuffer, 0x113);
    MILO_ASSERT(iBufSizeBytes > 0, 0x114);
    if (TheLoadMgr.GetPlatform() == kPlatformNone) {
        return false;
    } else {
        unkc = "";
        unk26 = b3;
        unk27 = b2;
        unkdc = i1;
        unk28 = b4;
        unk18 = true;
        unkb8 = kNoHandle;
        mAspect = 0;
        unkd6 = TheBinkMovieSys.GetUnkC();
        unk50.Reset();
        MILO_ASSERT(!mLoader, 0x127);
        MILO_ASSERT(!mBink, 0x128);
        MILO_ASSERT(!mPreloadBuf, 0x129);
        mPreloadBuf = iBuffer;
        unk20 = iBufSizeBytes;
        unk24 = b1;
        unkd4 = true;
        sActiveMovies.push_back(this);
        sActivePending++;
        return true;
    }
}

void BinkMovieImpl::Save(BinStream *stream) {
    MILO_ASSERT(stream, 0x1C7);
    if (stream->Cached()) {
        while (CheckOpen(false)) {
            TheLoadMgr.Poll();
        }
        FileLoader::SaveData(*stream, mPreloadBuf, unk20);
    }
}

#pragma endregion
