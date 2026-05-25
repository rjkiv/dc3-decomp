#pragma once
#include "movie/MovieImpl_p.h"

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
    virtual bool Paused() const; // 0x34 ?
    virtual void UnlockThread(); // 0x38
    virtual void LockThread(); // 0x3c
    virtual int GetFrame() const; // 0x40
    virtual float MsPerFrame() const; // 0x44
    virtual int NumFrames() const; // 0x48
    virtual void SetVolume(float); // 0x4c

    void Terminate();
};
