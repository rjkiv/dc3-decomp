#pragma once
#include "bink.h"
#include "movie/MovieImpl_p.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Str.h"
#include "xdk/win_types.h"

// size 0xC0
struct MovieInternalBuffers {
public:
    ~MovieInternalBuffers();
    static MovieInternalBuffers *New(std::vector<BINK *>);

private:
    MovieInternalBuffers();

    RndTex *YTex[2][2]; // 0x0
    RndTex *CrTex[2][2]; // 0x10
    RndTex *CbTex[2][2]; // 0x20
    RndTex *ATex[2][2]; // 0x30
    RndMat *unk40; // 0x40
    BINKFRAMEBUFFERS mBuffers; // 0x44
    int unkbc; // 0xbc
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
    virtual bool IsOpen(); // 0x24
    virtual bool IsLoading(); // 0x28
    virtual bool CheckOpen(bool); // 0x2c
    virtual bool SetPaused(bool); // 0x30
    virtual bool Paused() const { return mPaused; } // 0x34
    virtual void UnlockThread(); // 0x38
    virtual void LockThread(); // 0x3c
    virtual int GetFrame() const; // 0x40
    virtual float MsPerFrame() const; // 0x44
    virtual int NumFrames() const; // 0x48
    virtual void SetVolume(float); // 0x4c

    void Terminate();
    void DiscContentionCheck(Loader *);

private:
    bool PlatformCacheFile(const char *);

    static int sActivePending;
    static BinkMovieImpl *sAsyncMovie;
    static std::vector<BinkMovieImpl *> sActiveMovies;

    FileLoader *mLoader; // 0x4
    class BinkMovieLoader *mMovieLoader; // 0x8
    String unkc;
    BINK *mBink; // 0x14
    bool unk18;
    void *mPreloadBuf; // 0x1c
    int unk20;
    bool unk24;
    bool unk25;
    bool unk26;
    bool unk27;
    bool unk28;
    float mAspect; // 0x2c
    float unk30;
    float unk34;
    float unk38;
    float unk3c;
    int unk40;
    int mWidth; // 0x44
    int mHeight; // 0x48
    bool mPaused; // 0x4c
    Timer unk50;
    Timer unk80;
    int unkb0;
    int unkb4;
    void *unkb8;
    std::map<void *, String> unkbc;
    bool unkd4;
    bool unkd5;
    bool unkd6;
    DWORD mThreadId; // 0xd8
    int unkdc;
    int unke0;
    int unke4; // 0xe4 - MovieInternalBuffers*
};
