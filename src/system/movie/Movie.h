#pragma once
#include "MovieImpl.h"
#include "MovieSys.h"
#include "synth/Faders.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

class Movie {
public:
    Movie();
    ~Movie();
    static void Init();
    static void Terminate();
    static void Validate();
    void Save(BinStream *);
    void End();
    bool IsOpen() const;
    bool IsLoading() const;
    bool CheckOpen(bool);
    bool Ready() const;
    void SetPaused(bool);
    void UnlockThread();
    void LockThread();
    int GetFrame() const;
    float MsPerFrame() const;
    int NumFrames() const;
    void SetVolume(float);
    static int LocalizationTrack();
    bool BeginFromFile(
        char const *, float, bool, bool, bool, bool, int, BinStream *, LoaderPos
    );
    void Draw();
    bool Poll();
    void SetWidthHeight(int, int);

    FaderGroup *mFaderGroup; // 0x0
    MovieImpl *mImpl; // 0x4
};
