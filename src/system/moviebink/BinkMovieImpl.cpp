#include "moviebink/BinkMovieImpl.h"
#include "bink.h"
#include "math/Decibels.h"
#include "math/Utl.h"
#include "movie/MovieSys.h"
#include "moviebink/BinkMovieSys.h"
#include "moviebink/BinkMovieLoader.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirUnloader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Block.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OSFuncs.h"
#include "os/Platform.h"
#include "os/System.h"
#include "os/ThreadCall.h"
#include "os/Timer.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Mat.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "xdk/XAPILIB.h"

std::vector<BinkMovieImpl *> BinkMovieImpl::sActiveMovies;

namespace {
    void StoreCache(RndTex *t) {
        MILO_ASSERT(t, 0x3C);
        unsigned int pitch = t->TexelsPitch() * t->Height();
        void *buf;
        if (t->TexelsLock(buf)) {
            TheBinkMovieSys.PlatformStoreCache(buf, pitch);
            t->TexelsUnlock();
        }
    }

    void EndianSwapBuffer(void *buffer, int size);
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
    unk40 = nullptr;
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
            BINKFRAMEPLANESET &curFrame = ret->mBuffers.Frames[i];
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
                curFrame.YPlane.BufferPitch = ret->YTex[i][j]->TexelsPitch();
                curFrame.cRPlane.BufferPitch = ret->CrTex[i][j]->TexelsPitch();
                curFrame.cBPlane.BufferPitch = ret->CbTex[i][j]->TexelsPitch();
                curFrame.APlane.BufferPitch = ret->ATex[i][j]->TexelsPitch();

                if (ret->YTex[i][j]->TexelsLock(curFrame.YPlane.Buffer)) {
                    ret->YTex[i][j]->TexelsUnlock();
                }
                if (ret->CrTex[i][j]->TexelsLock(curFrame.cRPlane.Buffer)) {
                    ret->CrTex[i][j]->TexelsUnlock();
                }
                if (ret->CbTex[i][j]->TexelsLock(curFrame.cBPlane.Buffer)) {
                    ret->CbTex[i][j]->TexelsUnlock();
                }
                if (ret->ATex[i][j]->TexelsLock(curFrame.APlane.Buffer)) {
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
        MILO_ASSERT_FMT(                                                                 \
            mThreadId == CurrentThreadId() || (mThreadId == -1 && MainThread()),         \
            "%s called in the wrong thread (expected %d, cur thread is %d)",             \
            __FUNCTION__,                                                                \
            mThreadId,                                                                   \
            CurrentThreadId()                                                            \
        );                                                                               \
    }

BinkMovieImpl::BinkMovieImpl()
    : mLoader(0), mMovieLoader(0), mBink(0), unk18(0), mPreloadBuf(0), mBufferSize(0),
      unk24(0), unk40(0), mWidth(0), mHeight(0), mPaused(0), unkb8(kNoHandle), unkd4(0),
      unkd5(0), mThreadId(gMainThreadID), mVolume(0x8000), mInternalBufs(0) {
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
        mName = c1;
        unk18 = b3;
        if (!PlatformCacheFile(c1)) {
            return false;
        } else {
            unk26 = b2;
            unk27 = b1;
            mLocalizationTrack = i5;
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
                const char *str = mName.c_str();
                BinStream *bs6 = nullptr;
                if (bs && bs->Cached()) {
                    bs6 = bs;
                }
                mLoader = new FileLoader(str, str, pos, 0, true, true, bs6, "misc");
            } else {
                mMovieLoader = new BinkMovieLoader(mName.c_str(), kLoadStayBack, this);
            }
            sActiveMovies.push_back(this);
            sActivePending++;
            if (sActivePending > 1 && !b3) {
                MILO_NOTIFY("%s, multiple movies must be preloaded", mName);
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
        mName = "";
        unk26 = b3;
        unk27 = b2;
        mLocalizationTrack = i1;
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
        mBufferSize = iBufSizeBytes;
        unk24 = b1;
        unkd4 = true;
        sActiveMovies.push_back(this);
        sActivePending++;
        return true;
    }
}

void BinkMovieImpl::Draw() {
    CHECK_THREAD;
    if (!mBink) {
        TheNgRnd.Clear(1, Hmx::Color(0, 0, 0, 0));
    } else {
        unsigned int flags = mBink->OpenFlags;
        // mInternalBufs->unk40->SetNormalMap()
        SetRect();
        TheNgRnd.DrawRect(
            unk30,
            mInternalBufs->unk40,
            kMovieShader,
            Hmx::Color(0, 0, 0, 0),
            nullptr,
            nullptr
        );
        TheShaderMgr.SetPConstant((PShaderConstant)0, (RndTex *)nullptr);
        TheShaderMgr.SetPConstant((PShaderConstant)2, (RndTex *)nullptr);
        TheShaderMgr.SetPConstant((PShaderConstant)3, (RndTex *)nullptr);
        TheShaderMgr.SetPConstant((PShaderConstant)1, (RndTex *)nullptr);
    }
}

bool BinkMovieImpl::Poll() {
    CHECK_THREAD;
    if (CheckOpen(true)) {
        return true;
    } else if (mBink && mInternalBufs) {
        if (!mPreloadBuf) {
            float ms = unk50.SplitMs();
            unk50.Restart();
            if (ms > 49.0f) {
                const char *msg = MakeString(
                    "GLITCH: %g ms (%g ms bink), %s\n", ms, unk80.SplitMs(), mName
                );
                static DataNode &n = DataVariable("notify_level");
                if (n.Int() == 0) {
                    MILO_LOG("%s\n", msg);
                } else {
                    static Hmx::Object *sCheatDisplay =
                        ObjectDir::Main()->Find<Hmx::Object>("cheat_display");
                    static Message show("show", 0);
                    show[0] = msg;
                    sCheatDisplay->Handle(show, false);
                }
            }
            DiscContentionCheck(nullptr);
        }
        unk80.Restart();
        if (BinkWait(mBink) == 0) {
            DoFrame();
            while (BinkShouldSkip(mBink)) {
                MILO_LOG("skipped bink frame!\n");
                DoFrame();
            }
        }
        unk80.Stop();
        if (mBink->ReadError == 0 && (unk26 || mBink->FrameNum != mBink->Frames)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void BinkMovieImpl::Save(BinStream *stream) {
    MILO_ASSERT(stream, 0x1C7);
    if (stream->Cached()) {
        while (CheckOpen(false)) {
            TheLoadMgr.Poll();
        }
        FileLoader::SaveData(*stream, mPreloadBuf, mBufferSize);
    }
}

void BinkMovieImpl::End() {
    CHECK_THREAD;
    if (unkd4) {
        unkd4 = false;
        SharedFinishOpen(false);
    }
    FOREACH (it, sActiveMovies) {
        if (*it == this) {
            sActiveMovies.erase(it);
            break;
        }
    }
    if (!mPreloadBuf) {
        DataArray *videoArr = SystemConfig()->FindArray("videos", false);
        if (videoArr) {
            DataArray *streamArr = videoArr->FindArray("stream_end", false);
            if (streamArr) {
                streamArr->ExecuteScript(1, nullptr, nullptr, 1);
            }
        }
        DiscContentionPublish();
    }
    if (mBink) {
        MovieClose();
    }
    RELEASE(mLoader);
    RELEASE(mMovieLoader);
    if (unk24) {
        if (mPreloadBuf) {
            MemFree(mPreloadBuf, __FILE__, 0x212);
            mPreloadBuf = nullptr;
        }
    } else {
        mPreloadBuf = nullptr;
    }
    unk24 = false;
    if (mInternalBufs) {
        mInternalBufs->unkbc--;
        if (mInternalBufs->unkbc == 0) {
            delete mInternalBufs;
        }
        mInternalBufs = nullptr;
    }
    mThreadId = gMainThreadID;
}

bool BinkMovieImpl::IsOpen() const {
    CHECK_THREAD;
    return mBink;
}

bool BinkMovieImpl::IsLoading() const {
    CHECK_THREAD;
    return mLoader || mMovieLoader;
}

bool BinkMovieImpl::CheckOpen(bool b1) {
    if (!unkd4) {
        return false;
    }
    if (mLoader) {
        MILO_ASSERT(!mPreloadBuf, 0x24E);
        if (!mLoader->IsLoaded()) {
            return true;
        }
        unkd4 = false;
        mPreloadBuf = mLoader->GetBuffer(nullptr);
        mBufferSize = mLoader->GetSize();
        unk24 = true;
        RELEASE(mLoader);
        if (!mPreloadBuf) {
            SharedFinishOpen(b1);
            End();
            return false;
        }
        if (strneq((const char *)mPreloadBuf, "BIKi", 4)) {
            EndianSwapBuffer(mPreloadBuf, mBufferSize);
        }
        MovieOpen((const char *)mPreloadBuf, 0x4000400);
    } else if (mPreloadBuf) {
        unkd4 = false;
        if (strneq((const char *)mPreloadBuf, "BIKi", 4)) {
            EndianSwapBuffer(mPreloadBuf, mBufferSize);
        }
        MovieOpen((const char *)mPreloadBuf, 0x4000400);
    } else if (mMovieLoader && !mBink) {
        if (mMovieLoader->IsLoaded()) {
            unkd4 = false;
            DataArray *videoArr = SystemConfig()->FindArray("videos", false);
            if (videoArr) {
                DataArray *streamArr = videoArr->FindArray("stream_begin", false);
                if (streamArr) {
                    streamArr->ExecuteScript(1, nullptr, nullptr, 1);
                }
            }
            if (!UsingCD()) {
                MovieOpen(mName.c_str(), 0x400);
            } else {
                File *file = NewFile(mName.c_str(), FILE_OPEN_READ);
                if (file && file->GetFileHandle(unkb8)) {
                    MovieOpen((const char *)unkb8, 0x800400);
                }
            }
        } else {
            return true;
        }
    } else {
        return false;
    }
    SharedFinishOpen(b1);
    return false;
}

void BinkMovieImpl::SetPaused(bool paused) {
    if (mPaused != paused && mBink) {
        if (!paused) {
            LockThread();
        }
        if (unkd6 && paused && unkd5) {
            BinkDoFrameAsyncWait(mBink, -1);
            EndFrame();
        }
        BinkPause(mBink, paused);
        if (unkd6 && !paused && !unkd5) {
            BeginFrame();
            BinkDoFrameAsync(mBink, TheBinkMovieSys.Core0(), TheBinkMovieSys.Core1());
        }
        mPaused = paused;
        if (paused) {
            UnlockThread();
        }
    }
}

void BinkMovieImpl::UnlockThread() {
    MILO_ASSERT(mThreadId == CurrentThreadId(), 0x321);
    mThreadId = kNoThread;
}

void BinkMovieImpl::LockThread() {
    MILO_ASSERT(mThreadId == kNoThread, 0x32C);
    mThreadId = CurrentThreadId();
}

int BinkMovieImpl::GetFrame() const {
    CHECK_THREAD;
    if (mBink) {
        if (mBink->FrameNum == 1) {
            return mBink->Frames;
        } else {
            return mBink->FrameNum - 1;
        }
    } else {
        return 0;
    }
}

float BinkMovieImpl::MsPerFrame() const {
    CHECK_THREAD;
    if (mBink) {
        return (mBink->FrameRateDiv * 1000.0f) / (float)mBink->FrameRate;
    } else {
        return 0;
    }
}

int BinkMovieImpl::NumFrames() const {
    CHECK_THREAD;
    if (mBink) {
        return mBink->Frames;
    } else {
        return 0;
    }
}

void BinkMovieImpl::SetVolume(float db) {
    CHECK_THREAD;
    int ratio = DbToRatio(db) * 32768.0f;
    mVolume = ratio <= 0x8000 ? Max(ratio, 0) : 0x8000;
    if (mBink) {
        BinkSetVolume(mBink, 0, mVolume);
    }
}

void BinkMovieImpl::Terminate() {
    CHECK_THREAD;
    if (mBink) {
        MovieClose();
    }
}

void BinkMovieImpl::NextFrame() {
    CHECK_THREAD;
    unk40++;
    if (unk40 >= mInternalBufs->mBuffers.TotalFrames * TheBinkMovieSys.GetUnk10()) {
        unk40 = 0;
    }
    BinkNextFrame(mBink);
}

void BinkMovieImpl::DoFrame() {
    CHECK_THREAD;
    TheBlockMgr.MarkDiscRead();
    if (unkd6) {
        BinkDoFrameAsyncWait(mBink, -1);
        EndFrame();
        bool b3 = mBink->ReadError != 0 || !unk26 && mBink->FrameNum == mBink->Frames;
        if (!b3) {
            NextFrame();
            BeginFrame();
            BinkDoFrameAsync(mBink, TheBinkMovieSys.Core0(), TheBinkMovieSys.Core1());
        }
    } else {
        BeginFrame();
        BinkDoFrame(mBink);
        EndFrame();
        bool b3 = mBink->ReadError != 0 || !unk26 && mBink->FrameNum == mBink->Frames;
        if (!b3) {
            NextFrame();
        }
    }
}

void BinkMovieImpl::DiscContentionCheck(Loader *l) {
    CHECK_THREAD;
    FOREACH (it, TheLoadMgr.Loading()) {
        if (*it != l && !dynamic_cast<DirUnloader *>(*it)) {
            unkbc[*it] = (*it)->LoaderFile();
        }
    }
}

void BinkMovieImpl::DiscContentionPublish() {
    CHECK_THREAD;
    bool first = true;
    int count = 0;
    String str;
    FOREACH (it, unkbc) {
        if (!first) {
            str += ", ";
        }
        first = false;
        str += it->second;
        count++;
    }
    if (count != 0) {
        if (str.length() > 500) {
            str = str.substr(0, 500);
        }
        MILO_NOTIFY("Streaming Bink Thrashed with %d files: (%s)", count, str);
        unkbc.clear();
    }
}

void BinkMovieImpl::SharedFinishOpen(bool b1) {
    sActivePending--;
    MILO_ASSERT(sActivePending >= 0, 0x54B);
    if (sActivePending <= 0) {
        std::vector<BinkMovieImpl *> movies;
        std::vector<BINK *> binks;
        for (int i = 0; i < sActiveMovies.size(); i++) {
            BinkMovieImpl *impl = sActiveMovies[i];
            if (!impl->mInternalBufs) {
                movies.push_back(impl);
                binks.push_back(impl->mBink);
            }
        }
        MovieInternalBuffers *mibs = MovieInternalBuffers::New(binks);
        if (mibs) {
            for (int i = 0; i < movies.size(); i++) {
                BinkMovieImpl *impl = movies[i];
                impl->mInternalBufs = mibs;
                mibs->unkbc++;
                impl->FinishOpen();
            }
        }
        if (b1 && movies.size() == 1) {
            movies[0]->SetPaused(false);
        }
    }
}

void BinkMovieImpl::MovieOpen(const char *name, unsigned int flags) {
    CHECK_THREAD;
    MILO_ASSERT(!mBink, 0x408);
    mPaused = false;
    if (TheMovieSys.IsInitialized()) {
        if (mLocalizationTrack != 0) {
            if (TheBinkMovieSys.Track()) {
                mLocalizationTrack = TheBinkMovieSys.Track();
            }
            unsigned int tracks = mLocalizationTrack - 1;
            BinkSetSoundTrack(1, &tracks);
            flags |= 0x4000;
        }
        flags |= 0x100000;
        if ((flags >> 26) & 1) {
            AutoSlowFrame frame("BinkOpen", 200);
            mBink = BinkOpen(name, flags);
        } else {
            mBink = BinkOpen(name, flags);
        }
        if (mBink) {
            if (mLocalizationTrack != 0 && mBink->NumTracks != 0) {
                MILO_ASSERT(mLocalizationTrack - 1 < mBink->NumTracks, 0x42D);
            }
            BinkSetVolume(mBink, 0, mVolume);
            TheBinkMovieSys.AddMovie(this);
        }
    }
}

void BinkMovieImpl::MovieClose() {
    CHECK_THREAD;
    if (unkd6 && unkd5) {
        BinkDoFrameAsyncWait(mBink, -1);
        EndFrame();
    }
    TheBinkMovieSys.RemoveMovie(this);
    BinkClose(mBink);
    mBink = nullptr;
}

void BinkMovieImpl::FinishOpen() {
    CHECK_THREAD;
    if (!mBink) {
        MILO_NOTIFY("BinkOpen \'%s\' error: %s", mName, BinkGetError());
    } else {
        BinkSetSoundOnOff(mBink, unk27 == 0);
        BINKSUMMARY summary;
        BinkGetSummary(mBink, &summary);
        mAspect = float(summary.Width) / summary.Height;
        int width_low_4 = summary.Width % 16;
        if (summary.Height % 16 != 0 || width_low_4 != 0) {
            int height_low_4 = summary.Height % 16;
            int width_fixed = summary.Width;
            width_fixed += width_low_4 != 0 ? 16 - width_low_4 : 0;
            int height_fixed = summary.Height;
            height_fixed += height_low_4 != 0 ? 16 - height_low_4 : 0;
            MILO_FAIL(
                "Bink movie %s must have multiples of 16 for its width and height.\nTry changing from %d x %d to %d x %d.",
                mName.c_str(),
                summary.Width,
                summary.Height,
                width_fixed,
                height_fixed
            );
        }
        SetRect();
        SetPaused(true);
    }
}

#pragma endregion
