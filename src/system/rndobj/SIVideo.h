#pragma once
#include "utl/BinStream.h"

class SIVideo {
public:
    SIVideo() : mData(nullptr) { Reset(); }
    ~SIVideo() { Reset(); }
    void Reset();
    void Load(BinStream &, bool);
    int Bpp() const;
    int FrameSize() const;
    char *Frame(int);
    int Width() const { return mWidth; }
    int Height() const { return mHeight; }
    int NumFrames() const { return mNumFrames; }

    MEM_OVERLOAD(SIVideo, 5)

private:
    unsigned int mWidth; // 0x0
    unsigned int mHeight; // 0x4
    unsigned int mNumFrames; // 0x8
    unsigned int mOrder; // 0xc
    char *mData; // 0x10
};
