#pragma once
#include "math/Geo.h"
#include "rndobj/Cam.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

class RndBitmap;

class HiResScreen {
public:
    class BmpCache {
    public:
        BmpCache(unsigned int nWidth, unsigned int nHeight);
        virtual ~BmpCache();

        MEM_OVERLOAD(BmpCache, 0x6b)

        void DeleteCache();
        void GetLoadedRange(uint &, uint &) const;
        void LoadCache(uint pixelRowIndex);
        void FlushCache();
        void GetPixelColor(
            int x,
            int y,
            unsigned char &r,
            unsigned char &g,
            unsigned char &b,
            unsigned char &a
        ) const;
        void SetPixelColor(
            int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a
        );

        String *mFileNames; // 0x4
        unsigned char *mBuffer; // 0x8
        unsigned int mPixelsPerRow; // 0xc
        unsigned int mRowsPerCacheLine; // 0x10
        unsigned int mTotalNumCacheLines; // 0x14
        unsigned int mTotalRows; // 0x18
        unsigned int mCurrLoadedIndex; // 0x1c
        unsigned int mByteSize; // 0x20
        unsigned int mDirtyStart; // 0x24
        unsigned int mDirtyEnd; // 0x28
    };

    HiResScreen()
        : mActive(0), mTiling(3), mFileBase("urhigh"), mAccumWidth(0), mAccumHeight(0),
          mCurrTile(0), mEvenOddDisabled(0), mShrinkToSafe(1), mConsoleShowing(0),
          mCache(nullptr) {}
    virtual ~HiResScreen() {}
    void TakeShot(const char *fileBase, int tiling);
    void GetBorderForTile(
        int tileX,
        int tileY,
        int &borderLeft,
        int &borderTop,
        int &borderRight,
        int &borderBottom
    ) const;
    void Accumulate();
    void Finish();
    int GetPaddingX() const;
    int GetPaddingY() const;
    void
    CurrentTileRect(const Hmx::Rect &srcRect, Hmx::Rect &ratio, Hmx::Rect &outRect) const;
    Hmx::Rect ScreenRect(const RndCam *cam, const Hmx::Rect &rect) const;
    Hmx::Rect ScreenRect() const;
    Hmx::Rect InvScreenRect() const;
    bool Active() const { return mActive; }
    int Tiling() const { return mTiling; }

protected:
    void Merge(const RndBitmap &, int, int, int, int, int, int, int, int);
    void DownSample(RndBitmap &);

private:
    bool mActive; // 0x4
    int mTiling; // 0x8
    String mFileBase; // 0xC
    unsigned int mAccumWidth; // 0x14
    unsigned int mAccumHeight; // 0x18
    int mCurrTile; // 0x1c
    bool mOverride; // 0x20
    bool mEvenOddDisabled; // 0x21
    bool mShrinkToSafe; // 0x22
    bool mConsoleShowing; // 0x23
    BmpCache *mCache; // 0x24
};

extern HiResScreen &TheHiResScreen;
