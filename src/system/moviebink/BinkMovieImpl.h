#pragma once
#include "bink.h"
#include "math/Geo.h"
#include "movie/MovieImpl_p.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Str.h"
#include "xdk/win_types.h"

// size 0xC0
struct MovieInternalBuffers {
private:
    MovieInternalBuffers();

public:
    ~MovieInternalBuffers();
    static MovieInternalBuffers *New(std::vector<BINK *>);

    RndTex *YTex[2][2]; // 0x0
    RndTex *CrTex[2][2]; // 0x10
    RndTex *CbTex[2][2]; // 0x20
    RndTex *ATex[2][2]; // 0x30
    RndMat *unk40; // 0x40
    BINKFRAMEBUFFERS mBuffers; // 0x44
    int unkbc; // 0xbc - ref count?
};

class BinkMovieImpl : public MovieImpl {
public:
    BinkMovieImpl();
    virtual ~BinkMovieImpl(); // 0x0
    virtual void SetWidthHeight(int, int); // 0x4
    virtual bool Ready() const; // 0x8
    virtual bool BeginFromFile(
        char const *, float, bool, bool, bool, bool, int, BinStream *, LoaderPos
    ); // 0xc
    virtual bool BeginFromBuffer(void *, int, bool, float, bool, bool, bool, int); // 0x10
    virtual void Draw(); // 0x14
    virtual bool Poll(); // 0x18
    virtual void Save(BinStream *); // 0x1c
    virtual void End(); // 0x20
    virtual bool IsOpen() const; // 0x24
    virtual bool IsLoading() const; // 0x28
    virtual bool CheckOpen(bool); // 0x2c
    virtual void SetPaused(bool); // 0x30
    virtual bool Paused() const { return mPaused; } // 0x34
    virtual void UnlockThread(); // 0x38
    virtual void LockThread(); // 0x3c
    virtual int GetFrame() const; // 0x40
    virtual float MsPerFrame() const; // 0x44
    virtual int NumFrames() const; // 0x48
    virtual void SetVolume(float); // 0x4c

    void Terminate();
    void DiscContentionCheck(Loader *);
    void DiscContentionPublish();

private:
    bool PlatformCacheFile(const char *);

    void SetRect();
    void BeginFrame();
    void EndFrame();
    void NextFrame();
    void FinishOpen();
    void DoFrame();
    void MovieOpen(const char *, unsigned int);
    void MovieClose();

    static void SharedFinishOpen(bool);
    static int sActivePending;
    static BinkMovieImpl *sAsyncMovie;
    static std::vector<BinkMovieImpl *> sActiveMovies;

    FileLoader *mLoader; // 0x4
    class BinkMovieLoader *mMovieLoader; // 0x8
    /** The movie's name. */
    String mName; // 0xc
    BINK *mBink; // 0x14
    bool unk18;
    void *mPreloadBuf; // 0x1c
    int mBufferSize; // 0x20
    bool unk24;
    bool unk25;
    bool unk26;
    bool unk27;
    bool unk28;
    float mAspect; // 0x2c
    Hmx::Rect unk30; // 0x30
    int unk40;
    int mWidth; // 0x44
    int mHeight; // 0x48
    bool mPaused; // 0x4c
    Timer unk50;
    Timer unk80;
    int unkb0;
    int unkb4;
    void *unkb8; // 0xb8 - file handle, retrieved from mName
    std::map<void *, String> unkbc; // 0xbc - key = loader ptr, val = loader file?
    bool unkd4;
    bool unkd5; // 0xd5 - mMidFrame?
    bool unkd6;
    DWORD mThreadId; // 0xd8
    int mLocalizationTrack; // 0xdc
    int mVolume; // 0xe0
    MovieInternalBuffers *mInternalBufs; // 0xe4
};
