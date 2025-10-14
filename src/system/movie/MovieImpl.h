#pragma once
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

class MovieImpl {
public:
    virtual ~MovieImpl(); // 0x0
    virtual void SetWidthHeight(int, int) {} // 0x4 ?
    virtual bool Ready() const { return 0; } // 0x8
    virtual bool BeginFromFile(
        char const *, float, bool, bool, bool, bool, int, BinStream *, LoaderPos
    ); // 0xc
    virtual bool BeginFromBuffer(void *, int, bool, float, bool, bool, bool, int) {
        return 0;
    } // 0x10 ?
    virtual void Draw() {} // 0x14
    virtual bool Poll() { return 0; } // 0x18
    virtual void Save(BinStream *); // 0x1c
    virtual void End() {} // 0x20
    virtual bool IsOpen() { return 0; } // 0x24
    virtual bool IsLoading() { return 0; } // 0x28
    virtual bool CheckOpen(bool) { return 0; } // 0x2c
    virtual bool SetPaused(bool) { return 0; } // 0x30
    virtual bool Paused() const { return 0; } // 0x34 ?
    virtual void UnlockThread() {} // 0x38
    virtual void LockThread() {} // 0x3c
    virtual int GetFrame() const { return 0; } // 0x40
    virtual float MsPerFrame() const { return 1.0f; } // 0x44
    virtual int NumFrames() const { return 0; } // 0x48
    virtual void SetVolume(float) {} // 0x4c

    MEM_OVERLOAD(MovieImpl, 0x16);
};
