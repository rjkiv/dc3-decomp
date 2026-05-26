#include "moviebink/BinkMovieImpl.h"
#include "bink.h"
#include "moviebink/BinkMovieSys.h"
#include "moviebink/BinkMovieLoader.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OSFuncs.h"
#include "os/Platform.h"
#include "os/ThreadCall.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "xdk/XAPILIB.h"

std::vector<BinkMovieImpl *> BinkMovieImpl::sActiveMovies;

namespace {
    void StoreCache(RndTex *t);
    void EndianSwapBuffer(void *, int);
}

#pragma region MovieInternalBuffers

MovieInternalBuffers::MovieInternalBuffers() {
    memset(&mBuffers, 0, sizeof(BINKFRAMEBUFFERS));
    YTex[0][0] = nullptr;
    YTex[0][1] = nullptr;
    YTex[1][0] = nullptr;
    YTex[1][1] = nullptr;
    CrTex[0][0] = nullptr;
    CrTex[0][1] = nullptr;
    CrTex[1][0] = nullptr;
    CrTex[1][1] = nullptr;
    CbTex[0][0] = nullptr;
    CbTex[0][1] = nullptr;
    CbTex[1][0] = nullptr;
    CbTex[1][1] = nullptr;
    ATex[0][0] = nullptr;
    ATex[0][1] = nullptr;
    ATex[1][0] = nullptr;
    ATex[1][1] = nullptr;
    unk40 = 0;
    unkbc = 0;
}

MovieInternalBuffers::~MovieInternalBuffers() {
    RELEASE(unk40);
    for (uint i = 0; i < 2; i++) {
        for (uint j = 0; j < 2; j++) {
            RELEASE(YTex[i][j]);
            RELEASE(CrTex[i][j]);
            RELEASE(CbTex[i][j]);
            RELEASE(ATex[i][j]);
        }
    }
}

MovieInternalBuffers *MovieInternalBuffers::New(std::vector<BINK *> binks) {
    MovieInternalBuffers *ret = new MovieInternalBuffers();

    for (int i = 0; i < binks.size(); i++) {
        BINK *cur = binks[i];
        if (cur) {
            BINKFRAMEBUFFERS curBuffers;
            memset(&curBuffers, 0, sizeof(BINKFRAMEBUFFERS));
            BinkGetFrameBuffersInfo(cur, &curBuffers);
            ret->mBuffers.TotalFrames =
                Max(curBuffers.TotalFrames, ret->mBuffers.TotalFrames);
            ret->mBuffers.YABufferWidth =
                Max(curBuffers.YABufferWidth, ret->mBuffers.YABufferWidth);
            ret->mBuffers.YABufferHeight =
                Max(curBuffers.YABufferHeight, ret->mBuffers.YABufferHeight);
            ret->mBuffers.cRcBBufferWidth =
                Max(curBuffers.cRcBBufferWidth, ret->mBuffers.cRcBBufferWidth);
            ret->mBuffers.cRcBBufferHeight =
                Max(curBuffers.cRcBBufferHeight, ret->mBuffers.cRcBBufferHeight);
            BinkRegisterFrameBuffers(cur, &ret->mBuffers);
        }
    }
    if (ret->mBuffers.TotalFrames == 0) {
        delete ret;
        return nullptr;
    } else {
        for (int i = 0; i < ret->mBuffers.TotalFrames; i++) {
            for (int j = 0; j < TheBinkMovieSys.GetUnk10(); j++) {
                MILO_ASSERT(!ret->YTex[i][j], 0x5A2);
                MILO_ASSERT(!ret->CrTex[i][j], 0x5A3);
                MILO_ASSERT(!ret->CbTex[i][j], 0x5A4);
                MILO_ASSERT(!ret->ATex[i][j], 0x5A5);
                ret->YTex[i][j] = Hmx::Object::New<RndTex>();
                ret->CrTex[i][j] = Hmx::Object::New<RndTex>();
                ret->CbTex[i][j] = Hmx::Object::New<RndTex>();
                ret->ATex[i][j] = Hmx::Object::New<RndTex>();
                ret->YTex[i][j]->SetBitmap(
                    ret->mBuffers.YABufferWidth,
                    ret->mBuffers.YABufferHeight,
                    8,
                    (RndTex::Type)0x24,
                    false,
                    nullptr
                );
                ret->CrTex[i][j]->SetBitmap(
                    ret->mBuffers.cRcBBufferWidth,
                    ret->mBuffers.cRcBBufferHeight,
                    8,
                    (RndTex::Type)0x24,
                    false,
                    nullptr
                );
                ret->CbTex[i][j]->SetBitmap(
                    ret->mBuffers.cRcBBufferWidth,
                    ret->mBuffers.cRcBBufferHeight,
                    8,
                    (RndTex::Type)0x24,
                    false,
                    nullptr
                );
                ret->ATex[i][j]->SetBitmap(
                    ret->mBuffers.YABufferWidth,
                    ret->mBuffers.YABufferHeight,
                    8,
                    (RndTex::Type)0x24,
                    false,
                    nullptr
                );
                ret->mBuffers.Frames[i].YPlane.BufferPitch =
                    ret->YTex[i][j]->TexelsPitch();
                ret->mBuffers.Frames[i].cRPlane.BufferPitch =
                    ret->CrTex[i][j]->TexelsPitch();
                ret->mBuffers.Frames[i].cBPlane.BufferPitch =
                    ret->CbTex[i][j]->TexelsPitch();
                ret->mBuffers.Frames[i].APlane.BufferPitch =
                    ret->ATex[i][j]->TexelsPitch();

                if (ret->YTex[i][j]->TexelsLock(ret->mBuffers.Frames[i].YPlane.Buffer)) {
                    ret->YTex[i][j]->TexelsUnlock();
                }
                if (ret->CrTex[i][j]->TexelsLock(ret->mBuffers.Frames[i].cRPlane.Buffer)) {
                    ret->CrTex[i][j]->TexelsUnlock();
                }
                if (ret->CbTex[i][j]->TexelsLock(ret->mBuffers.Frames[i].cBPlane.Buffer)) {
                    ret->CbTex[i][j]->TexelsUnlock();
                }
                if (ret->ATex[i][j]->TexelsLock(ret->mBuffers.Frames[i].APlane.Buffer)) {
                    ret->ATex[i][j]->TexelsUnlock();
                }
            }
        }

        ret->unk40 = Hmx::Object::New<RndMat>();
        ret->unk40->SetPreLit(true);
        ret->unk40->SetUseEnv(false);
        ret->unk40->SetBlend(RndMat::kBlendSrc);
        ret->unk40->SetAlphaWrite(false);
        ret->unk40->SetZMode(kZModeDisable);
        ret->unk40->SetTexWrap(kTexWrapClamp);
        CreateAndSetMetaMat(ret->unk40);
        return ret;
    }
}

#pragma endregion
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
